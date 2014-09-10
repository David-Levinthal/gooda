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
         "dojo/data/ItemFileReadStore",
         "dijit/tree/ForestStoreModel",
         "dijit/layout/TabContainer",
         "dijit/layout/ContentPane",
         "dijit/Tree"], function(declare, 
                                 ItemFileReadStore,
                                 ForestStoreModel,
                                 TabContainer, 
                                 ContentPane,
                                 Tree){
  declare("GOoDA.HelpPane", GOoDA.ContainerView, {
  
    constructor: function(params){
      declare.safeMixin(this, params);
      
      var self = this;
      
      var dataStore = {
        identifier: 'name',
        label: 'name',
        items: [
          {name: 'Welcome'},
          {name: 'Tutorial'},
          {name: 'Cycle Accounting'}
        ]
      };
      
      var store = new ItemFileReadStore({
        data: dataStore
      });
      
      var model = new ForestStoreModel({
        store: store
      });
      
      var tree = new Tree({
        title: "Help",
        model: model,
        showRoot: false,
        persist: false,
        
        onClick: function(e){
          switch(e.name[0]){
            case 'Welcome':
              self.showWelcomeScreen();
              break;
            case 'Tutorial':
              self.showTutorialScreen();
              break;
            case 'Cycle Accounting':
              self.showCycleAccountingScreen();
              break;
            default:
              break;
          }
        }
      })
      
      this.container = new TabContainer({
      });
      
      this.parentContainer.addView(this.container);
      
      this.welcomePane = new ContentPane({
        title: 'Welcome',
        content: '<div class="help" id="tutorial">\
                  <p>Modern superscalar, out-of-order microprocessors dominate large scale server computing. Monitoring their activity, during program execution, has become complicated due to the complexity of the microarchitectures and their IO interactions. Recent processors have thousands of performance monitoring events. These are required to actually provide coverage for all of the complex interactions and performance issues that can occur. Knowing which data to collect and how to interpret the results has become an unreasonable burden for code developers whose tasks are already hard enough.  It becomes the task of the analysis tool developer to bridge this gap.</p>\
                  <img src="images/cycleTree.png" />\
                  <p>To address this issue, a generic decomposition of how a microprocessor is using the consumed cycles allows code developers to quickly understand which of the myriad of microarchitectural complexities they are battling, without requiring a detailed knowledge of the microarchitecture. When this approach is intrinsically integrated into a performance data analysis tool, it enables software developers to take advantage of the microarchitectural methodology that has only been available to experts.\
                  The Generic Optimization Data Analyzer (GOoDA) project integrates this expertise into a profiling tool in order to lower the required expertise of the user and, being designed from the ground up with large-scale object-oriented applications in mind, it will be particularly useful for large HENP codebases.</p></div>'
      });

      this.cycleAccountingPane = new ContentPane({
        title: 'Cycle Accounting',
        content: '<div class="help" id="cycleAccounting">\
                  <iframe src="http://docs.google.com/gview?url=http://gooda.googlecode.com/git/gooda-analyzer/docs/CycleAccountingandPerformanceAnalysis.pdf&embedded=true" style="width:100%; height:700px; display:block; margin-left:auto; margin-right: auto" frameborder="0"></iframe>'
      });

     this.tutorialPane = new ContentPane({
        title: 'Tutorial',
        content: '<div class="help" id="tutorial">\
                  <iframe src="http://docs.google.com/gview?url=https://github.com/vitillo/talks/blob/master/GoodaTutorial2012.pdf?raw=true&embedded=true" style="width:100%; height:700px; display:block; margin-left:auto; margin-right: auto" frameborder="0"></iframe>'
      });
      
      this.container.addChild(this.welcomePane);     
      this.container.addChild(this.tutorialPane);     
      this.container.addChild(this.cycleAccountingPane);
      this.navigationMenu.addChild(tree);
    },
    
    showWelcomeScreen: function(){
      this.highlight();
      this.highlightView(this.welcomePane);
    },

    showTutorialScreen: function(){
      this.highlight();
      this.highlightView(this.tutorialPane);
    },

    showCycleAccountingScreen: function(){
      this.highlight();
      this.highlightView(this.cycleAccountingPane);
    }
  })
});
