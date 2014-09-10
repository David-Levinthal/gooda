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
         "dojo/_base/lang",
         "dojo/aspect",
         "dijit/layout/BorderContainer"], function(declare,
                                                   lang,
                                                   aspect,
                                                   BorderContainer){
  declare("GOoDA.DataView", null, {
    constructor: function(params){
      var self = this;
      
      declare.safeMixin(this, params);
      
      this.fileLoader = new GOoDA.FileLoader();
      this.visualizer = GOoDA.Visualizer.getInstance();
      this.visualizer.showLoadScreen();
      this.container = new BorderContainer({
        title: this.title,
        onShow: function(){ self.refresh(); }
      });
    },

    loadResource: function(loader, store, params){
      var self = this;

      var args = {
        report: this.report.name,
        functionID: this.functionID,
        processID: this.processID,
        
        success: function(data){
          self[store] = data;
          self.buildView();
        },
        
        failure: function(){
          self[store] = null;
          self.buildView();
        }
      };
      
      lang.mixin(args, params);
      loader.call(this.fileLoader, args);
    },
    
    resourceProcessed: function(){
      if(--this.resourcesToLoad === 0){
        this.onLoaded && this.onLoaded();
        this.visualizer.hideLoadScreen();
      }
    },
    
    failure: function(msg){
      new GOoDA.Notification({title: 'Error', content: msg});
      this.visualizer.hideLoadScreen();
    },
    
    onResize: function(widget, wrapper){
      aspect.after(widget, 'resize', wrapper);
    },

    notify: function(ev){
      this[ev.id](ev.payload);
    }
  })
});
