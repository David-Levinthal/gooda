/*
  Visualizer for the Generic Optimization Data Analyzer, Copyright (c)
  2012, The Regents of the University of California, through Lawrence
  Berkeley National Laboratory (subject to receipt of any required
  approvals from the U.S. Dept. of Energy).  All rights reserved.
  
  This code is derived from software contributed by Roberto Agostino 
  Vitillo <ravitillo@lbl.gov>.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  (1) Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  (2) Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  (3) Neither the name of the University of California, Lawrence
  Berkeley National Laboratory, U.S. Dept. of Energy nor the names of
  its contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

require(["dojo/_base/declare",
         "dijit/layout/ContentPane",
         "dijit/layout/BorderContainer",
         "dojox/html/entities"], function(declare, 
                                          ContentPane, 
                                          BorderContainer,
                                          entities){
  declare("GOoDA.FunctionView", GOoDA.DataView, {
    constructor: function(params){
      var self = this;

      declare.safeMixin(this, params);
      
      this.resourcesToLoad = 3;
      
      this.loadResource(this.fileLoader.getASMData, 'ASMData');
      this.loadResource(this.fileLoader.getSRCData, 'SRCData');
      this.loadResource(this.fileLoader.getCFGData, 'CFGData');
    },
    
    buildView: function(){
      if(this.ASMData === undefined || this.SRCData === undefined || this.CFGData === undefined)
        return;

      this.buildASMPane();
      this.buildSRCPane();
      this.buildCFGPane();
    },
    
    buildASMPane: function(){
      var self = this;
      var title;
      var functionName;
      
      if(!this.ASMData){
        this.failure('Report data for selected function not available.');
        return;
      }
      
      functionName = this.ASMData.functionName;
      this.container.closable = true;
      this.container.name = this.report.name + this.functionID;
      this.container.title = (functionName.length > 25) 
                             ? this.container.title = '<div title="' + entities.encode(functionName) + '">' + entities.encode(functionName.substr(0, 22)) + '...</div>' 
                             : functionName;
      
      this.asmContainer = new BorderContainer({
        region: "center",
        style: "padding: 0",
        splitter: true
      });
      
      this.onResize(this.asmContainer, function(){
        self.asmView && self.asmView.resize();
      });

      this.container.addChild(this.asmContainer);
      this.container.goodaView = this;
      this.report.addView(this.container);
      
      if(this.hiddenColumns){
        for(var i = 0; i < this.ASMData.columns.length; i++){
          var c = this.ASMData.columns[i];

          if(this.hiddenColumns[c.name])
            c.visible = false;
        }
      }

      this.asmView = new GOoDA.EventTable({
        report: self.report,
        container: self.asmContainer,
        source: 'Disassembly',
        data: self.ASMData,
        tree: true,
        expand: true,
        
        rowCssClasses: function(d){ 
          return !d.parent ? " parent" : "";
        },

        hideColumnHandler: function(column){
          self.report.notifyViews({id: 'hideColumn', payload: column.name});
        },
        
        codeHandler: function(target, e, row, cell, item, table){           
          if(item['parent']) 
            return;

          table.selectRows([item]);
          self.cfgView && self.cfgView.select('node' + item[GOoDA.Columns.DISASSEMBLY].split(' ')[3]);
          self.srcView && self.srcView.select(function(row){
            return (row[GOoDA.Columns.LINENUMBER] === item[GOoDA.Columns.PRINCIPALLINENUMBER]);
          });
        },
      });

      this.sourceLines = this.ASMData.sourceLines;
      this.highlightedASMRow = this.ASMData.maxRow;
      this.resourceProcessed();
      delete this.ASMData;
    },
    
    buildSRCPane: function(){
      var self = this;
      
      if(!this.SRCData){
        this.resourceProcessed();
        return;
      }
      
      this.srcContainer = new BorderContainer({
        region: "right",
        splitter: true,
        style: "padding: 0; width: " + ($(this.container.domNode).width()/2 - 6) + "px;"
      });
      
      this.onResize(this.srcContainer, function(){
        self.srcView && self.srcView.resize();
      });
      
      this.container.addChild(this.srcContainer);
    
      if(this.hiddenColumns){
        for(var i = 0; i < this.SRCData.columns.length; i++){
          var c = this.SRCData.columns[i];

          if(this.hiddenColumns[c.name])
            c.visible = false;
        }
      }

      this.srcView = new GOoDA.EventTable({
        report: self.report,
        container: self.srcContainer,
        source: 'Source',
        data: self.SRCData,
        
        rowCssClasses: function(d){ return d.highlight ? " parent" : "";},
        hideColumnHandler: function(column){
          self.report.notifyViews({id: 'hideColumn', payload: column.name});
        },
        codeHandler: function(target, e, row, cell, item, table){
          if(!item.highlight) return;

          var selection = self.asmView.select(function(row){
            return !row.parent && row[GOoDA.Columns.PRINCIPALLINENUMBER] == item[GOoDA.Columns.LINENUMBER];
          }, true);
          
          if(selection){
            table.selectRows([item]);
            
            if(!self.cfgView) 
              return;
            
            for(var i = 0; i < selection.length; i++)
              self.cfgView.select('node' + selection[i][GOoDA.Columns.DISASSEMBLY].split(' ')[3], i !== 0);
          }
        }
      });
      
      this.sourceLines && this.srcView.eachRow(function(row){
        if(row[GOoDA.Columns.LINENUMBER] in self.sourceLines){
          row.highlight = true;
        }
      });
      
      this.resourceProcessed();
      delete this.SRCData;
    },
    
    buildCFGPane: function(){
      var self = this;
      
      if(!this.CFGData){
        this.resourceProcessed();
        return;
      }
      
      this.cfgContainer = new BorderContainer({
        style: "height: 33%; width: 100%; ",
        region: "bottom",
        splitter: true
      });

      this.onResize(this.cfgContainer, function(){
        self.cfgView && self.cfgView.resize();
      });
    
      this.container.addChild(this.cfgContainer);

      this.cfgView = new GOoDA.GraphPane({
        container: self.cfgContainer,
        svg: self.CFGData,
        
        clickHandler: function(bb){
          var selection = self.asmView.select(function(row){
            return !row.parent && row[GOoDA.Columns.DISASSEMBLY].indexOf('Basic Block ' + bb.substr(4)) != -1;
          });
          
          selection && self.srcView && self.srcView.select(function(row){
            return (row[GOoDA.Columns.LINENUMBER] === selection[GOoDA.Columns.PRINCIPALLINENUMBER])
          });

          return selection;
        }
      });
      
      this.resourceProcessed();
      delete self.CFGData;
    },
    
    unhideColumns: function(){
      this.asmView && this.asmView.unhideColumns();
      this.srcView && this.srcView.unhideColumns();
    },

    hideColumn: function(column){
      this.asmView && this.asmView.hideColumn(column);
      this.srcView && this.srcView.hideColumn(column);
    },

    onLoaded: function(){
      var item = this.highlightedASMRow;

      this.asmView && this.asmView.selectRows([item]);
      this.cfgView && this.cfgView.select('node' + item[GOoDA.Columns.DISASSEMBLY].split(' ')[3]);
      this.srcView && this.srcView.select(function(row){
          return (row[GOoDA.Columns.LINENUMBER] === item[GOoDA.Columns.PRINCIPALLINENUMBER]);
      });
      
      this.visualizer.gotoState({process: this.processID, 'function': this.functionID});
      this.state = this.visualizer.getState();
    },
    
    refresh: function(){
      this.asmView && this.asmView.refresh();
      this.srcView && this.srcView.refresh();
      this.state && this.visualizer.gotoState(this.state, true);
    }
  })
});
