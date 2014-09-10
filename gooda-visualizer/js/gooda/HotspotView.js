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
         "dojo/on",
         "dijit/layout/ContentPane",
         "dijit/layout/BorderContainer",
         "dojox/widget/Standby"], function(declare, 
                                           on,
                                           ContentPane, 
                                           BorderContainer,
                                           StandBy){
  declare("GOoDA.HotspotView", GOoDA.DataView, {
    constructor: function(params){
      var self = this;
    
      declare.safeMixin(this, params);
      
      this.hiddenColumns = {};
      this.resourcesToLoad = 3;
      this.selection = {
        processID: 0,
        id: 0
      }
      this.bottomContainer = new BorderContainer({
        region: "center",
        splitter: true,
        style: "padding: 0"
      });
      this.container.addChild(this.bottomContainer);
      
      this.loadResource(this.fileLoader.getHotProcessData, 'HotProcessData');
      this.loadResource(this.fileLoader.getHotFunctionData, 'HotFunctionData');
      this.loadResource(this.fileLoader.getCGData, 'CGData', {processID: 0});
    },
    
    buildView: function(){
      var self = this;

      if(this.HotProcessData === undefined || this.HotFunctionData === undefined || this.CGData === undefined)
        return;
      else if(this.HotProcessData === null || this.HotFunctionData === null){
        this.failure('Missing report data!');
        this.visualizer.unloadReport(this.report.name);
       }else{
        this.buildHotProcessView();
        this.buildHotFunctionView();
        this.buildCGView();
      }
    },
    
    buildHotProcessView: function(){
      var self = this;
      
      this.processContainer = new BorderContainer({
        region: "top",
        style: "height: 33%; padding: 0",
        splitter: true,
      });
      
      this.onResize(this.processContainer, function(){
        self.hotProcessView && self.hotProcessView.resize();
      })
      
      this.container.addChild(this.processContainer);
      this.container.goodaView = this;
      this.report.addView(this.container);
      
      this.hotProcessView = new GOoDA.EventTable({
        report: self.report,
        container: self.processContainer,
        source: GOoDA.Columns.PROCESSPATH,
        data: self.HotProcessData,
        expand: true,
        
        rowCssClasses: function(d){
          return !d.parent ? " parent" : "";
        },

        hideColumnHandler: function(column){
          self.report.notifyViews({id: 'hideColumn', payload: column.name});
        },
        
        codeHandler: function(target, e, row, cell, data, table){
          var criterias = {};
          var process;
          var paths;
          var module;
          var processID;
          var selectedFunction;

          if(!data)
            table.selectRows([]);
          else{
            processID = data.parent ? data.parent.id : data.id;
            process = data[GOoDA.Columns.PROCESSPATH] || data.parent[GOoDA.Columns.PROCESSPATH];
            if(data[GOoDA.Columns.MODULEPATH]){
              paths = data[GOoDA.Columns.MODULEPATH].split(/(\\|\/)/g);
              module = paths[paths.length - 1];
            }

            criterias[GOoDA.Columns.PROCESS] = process;
            criterias[GOoDA.Columns.MODULE] = module;
            
            self.unselect();
            table.selectRows([data]);

            if(self.hotFunctionView){
              self.hotFunctionView.filter(criterias);

              selectedFunction = self.hotFunctionView.select(function(row){
                return true;
              });

              selectedFunction && self.cgView && self.cgView.select(selectedFunction[GOoDA.Columns.FUNCTIONID]);
            }
            
            if(self.selection.processID != processID){
              if(self.cgLoadScreen){
                self.cgLoadScreen.show();
                self.loadResource(self.fileLoader.getCGData, 'CGData', {
                  processID: data.id,
                  success: function(data){
                    self._buildCGView(data, selectedFunction[GOoDA.Columns.FUNCTIONID]);
                    self.cgLoadScreen.hide();
                  },
                  failure: function(){
                    self.cgLoadScreen.hide();
                    self.cgView.empty();
                  }
                });
              }
              
              self.selection.processID = processID;
            }
          }
        }
        
      });
      
      this.hotProcessView.toggleExpansion();
      this.resourceProcessed();
      delete this.HotProcessData;
    },
    
    buildHotFunctionView: function(){
      var self = this;
      
      this.functionContainer = new BorderContainer({
        region: "center",
        splitter: true,
        style: "padding: 0"
      });
      
      this.onResize(this.functionContainer, function(){
        self.hotFunctionView && self.hotFunctionView.resize();
      })
      
      this.bottomContainer.addChild(this.functionContainer);
      
      this.hotFunctionView = new GOoDA.EventTable({
        report : self.report,
        container: self.functionContainer,
        source: GOoDA.Columns.FUNCTIONNAME,
        data: self.HotFunctionData,
        expand: true,
        
        hideColumnHandler: function(column){
          self.report.notifyViews({id: 'hideColumn', payload: column.name});
        },

        codeHandler: function(target, e, row, cell, data, table){
          table.selectRows([data]);
          self.cgView && self.cgView.select(data[GOoDA.Columns.FUNCTIONID]);
          self._openFunctionView(data);
        },
      });

      this.resourceProcessed();
      this.hotFunctionView.toggleExpansion();
      delete this.HotFunctionData;
    },
    
    _openFunctionView: function(data){
      var processName = data[GOoDA.Columns.PROCESS];
      var functionName = data[GOoDA.Columns.FUNCTIONNAME];
      var functionID = data[GOoDA.Columns.FUNCTIONID];
      var functionView = null;
      
      if((functionView = this.report.getView(this.report.name + functionID)))
        this.report.highlightView(functionView);
      else{
        new GOoDA.FunctionView({
          report: this.report,
          functionID: functionID,
          hiddenColumns: this.hiddenColumns
        });
      }
    },
    
    buildCGView: function(){
      var self = this;

      if(!this.CGData){
        this.resourceProcessed();
        return;
      }

      this.cgContainer = new BorderContainer({
        title: "Call Graph",
        style: "padding: 0; width: " + ($(this.bottomContainer.domNode).width()/2 - 2) + "px;",
        region: "right",
        splitter: true
      });

      this.onResize(this.cgContainer, function(){
        self.cgView && self.cgView.resize();
      });

      this.bottomContainer.addChild(this.cgContainer);
      
      this.cgLoadScreen = new dojox.widget.Standby({
        target: this.cgContainer.domNode,
        duration: 1,
        color: '#D3E1EB'
      });
      
      document.body.appendChild(this.cgLoadScreen.domNode);
      this.cgLoadScreen.startup();
      
      this._buildCGView(this.CGData);
      this.resourceProcessed();
      delete this.CGData;
    },
    
    _buildCGView: function(data, selection){
      var self = this;
      
      if(this.cgView){
        this.cgView.reload(data, selection);
      }else{
        this.cgView = new GOoDA.GraphPane({
          container: self.cgContainer,
          svg: data,
          clickHandler: function(funID){
            var found = false;
            
            self.hotFunctionView.select(function(row){
              if(!row['parent'] && row[GOoDA.Columns.FUNCTIONID] == funID){
                found = true;
                return true;
              }else
                return false;
            });

            return found;
          },
          altClickHandler: function(id){
            var grid = self.hotFunctionView.data.grid;
            
            for(var i = 0; i < grid.length; i++){
              if(!grid[i]['parent'] && grid[i][GOoDA.Columns.FUNCTIONID] == id){
                self._openFunctionView(grid[i]);
                return;
              }
            }
          }
        });
      }
    },

    unhideColumns: function(){
      this.hiddenColumns = {};
      this.hotProcessView && this.hotProcessView.unhideColumns();
      this.hotFunctionView && this.hotFunctionView.unhideColumns();
    },

    hideColumn: function(column){
      this.hiddenColumns[column] = true;
      this.hotProcessView && this.hotProcessView.hideColumn(column);
      this.hotFunctionView && this.hotFunctionView.hideColumn(column);
    },
    
    onLoaded: function(){
      this.hotProcessView && this.hotProcessView.select(function (row){
        return row.id === 0;        
      });
      
      this.hotFunctionView && this.hotFunctionView.select(function (row){
        return row.id === 0;        
      });
      
      this.cgView && this.cgView.select(0);
      
      this.visualizer.gotoState({report: this.report.name}, true);
      this.state = this.visualizer.getState();
    },
    
    unselect: function(){
      this.hotFunctionView.unselect();
      this.cgView && this.cgView.unselect();
    },
    
    refresh: function(){
      this.hotFunctionView && this.hotFunctionView.refresh();
      this.hotProcessView && this.hotProcessView.refresh();
      this.state && this.visualizer.gotoState(this.state, true);
    },
  })
});
