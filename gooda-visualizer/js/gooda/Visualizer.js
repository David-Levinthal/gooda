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

require(["dijit/layout/BorderContainer",
         "dijit/layout/AccordionContainer",
         "dijit/layout/StackContainer",
         "dijit/layout/ContentPane",
         "dijit/Tree",
         "dijit/tree/ForestStoreModel",
         "dijit/form/Button",
         "dojo/data/ItemFileWriteStore",
         "dojox/widget/Standby",
         "dojo/_base/declare"], function(BorderContainer,
                                         AccordionContainer, 
                                         StackContainer,
                                         ContentPane,
                                         Tree,
                                         ForestStoreModel,
                                         Button,
                                         ItemFileWriteStore,
                                         Standby,
                                         declare){
  declare("GOoDA.Visualizer", GOoDA.ContainerView, {
    constructor: function(params){
      var self = this;

      var source = {
        identifier: 'report', 
        label: 'report', 
        items: []
      };

      this.reportStore = new ItemFileWriteStore({
        identifier: 'report',
        data: source
      });

      this.fileLoader = new GOoDA.FileLoader();
      this.fileLoader.readFile("reports/index", function(data){
        var reports = data.split("\n");

        for(var i in reports){
          var report = reports[i];

          if(!report || report == "report"){
            continue;
          }

          try{
            self.reportStore.newItem({report: "reports/" + report});
          }catch(err){
            //report loaded through GET parameter
          }
        }
      }, function(){
      })
      
      var reportModel = new ForestStoreModel({
        store: this.reportStore
      });
            
      this.state = new GOoDA.State({
        visualizer: this
      });
      
      this.viewport = new BorderContainer({
      }, "viewport");
      
      this.header = new ContentPane({
        region: "top",
        id: 'headerContainer',
        content: "<div id='header'>Generic Optimization Data Analyzer</div>"
      });
      
      this.container = new StackContainer({
        region: 'center',
        splitter: true,
        style: "border: 0",
        id: 'workspace'
      });
      
      this.loadScreen = new dojox.widget.Standby({
          target: this.container.domNode,
          duration: 1,
          color: '#D3E1EB'
      });
  
      document.body.appendChild(this.loadScreen.domNode);

      this.navigationMenu = new AccordionContainer({
        region: 'left',
        splitter: true,
        id: "navigationMenu"
      });
      
      //TODO: find a better way to open directories without using a java applet
      this.reportExplorer = new Tree({
        title: "Reports",
        //title: "Reports <span id='loadButton' title='Open report from local drive' class='toggle expand' style='float: right; margin-top:3px'></span>",
        model: reportModel,
        showRoot: false,
        persist: false,
        
        onClick: function(e){
          self.loadReport(e.report);
        },
        /*onLoad: function(){
          $('#loadButton').click(function(){
            if(!jsFs || !jsFs.explore){
              new GOoDA.Notification({
                title: 'Warning', 
                content: 'Java applet not enabled!'
              });
            }else{
              var report = jsFs.explore();
              report && self.loadReport(report);
            }
          });
        }*/
      });

      // http://bugs.dojotoolkit.org/ticket/14554
      this.reportExplorer.containerNode = this.reportExplorer.domNode;
      
      this.navigationMenu.addChild(this.reportExplorer);
      this.viewport.addChild(this.navigationMenu);
      this.viewport.addChild(this.container);
      
      this.helpPane = new GOoDA.HelpPane({
        title: 'Help',
        parentContainer: this,
        navigationMenu: this.navigationMenu
      });
      
      this.viewport.startup();
      this.loadScreen.startup();
  
      if(!this.browserSupported())
        new GOoDA.Notification({
          title: 'Warning', 
          content: 'Browser version not supported, the application may not work as expected.'
        });
    },
    
    loadReport: function(name){
      GOoDA.Report.create(name);
    },

    unloadReport: function(name){
      var self = this;
    },
    
    loadFunction: function(reportName, functionID){
      var functionView;
      GOoDA.Report.create(reportName, function(report){
        if((functionView = report.getView(report.name + functionID)))
          report.highlightView(functionView);
        else{
          new GOoDA.FunctionView({
            report: report,
            functionID: functionID
          });
        }
      });
    },
    
    showLoadScreen: function(){
      this.loadScreen.show();
    },
    
    hideLoadScreen: function(){
      this.loadScreen.hide();
    },
    
    browserSupported: function(){
      if(!Modernizr.inlinesvg) return false;
      if(!Modernizr.csstransforms) return false;
      if(Modernizr.touch) return false;
      return true;
    },
    
    restoreState: function(){
      this.state.restoreState();
    },
    
    gotoState: function(state, fresh){
      this.state.gotoState(state, fresh);
    },
    
    getState: function(){
      return this.state.getState();
    }
  });

  GOoDA.Visualizer.instance = null;
  GOoDA.Visualizer.getInstance = function(){
    if(!GOoDA.Visualizer.instance)
      GOoDA.Visualizer.instance = new GOoDA.Visualizer();
    
    return GOoDA.Visualizer.instance;
  }
});
