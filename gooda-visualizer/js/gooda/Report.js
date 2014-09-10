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
         "dijit/layout/TabContainer"], function(declare,
                                                TabContainer){
  declare("GOoDA.Report", GOoDA.ContainerView, {
    constructor: function(params){
      declare.safeMixin(this, params);
      
      this.visualizer = GOoDA.Visualizer.getInstance();
      this.hotspotView = new GOoDA.HotspotView({
        title: this.name + ' Hotspots',
        report: this
      });
    },
        
    addView: function(view){
      var self = this;
      var firstLoad = false;
      
      if(!this.container){
        firstLoad = true;
        GOoDA.Report.reports[this.name] = this;
        this.container = new TabContainer({
          onShow: function(){
            self.container.selectedChildWidget && self.container.selectedChildWidget.onShow();
          }
        });
        this.visualizer.addView(this.container);
        this.visualizer.highlightView(this.container);
      }
      
      this.container.addChild(view);
      this.container.selectChild(view);
      firstLoad && this.onLoadHandler && this.onLoadHandler(this);
    },

    notifyViews: function(ev){
      var children = this.container.getChildren();

      for(var i = 0; i < children.length; i++)
        children[i].goodaView.notify(ev);
    }
  });
  
  GOoDA.Report.reports = {};
  GOoDA.Report.create = function(name, onLoadHandler){
    var report = GOoDA.Report.reports[name];
    var visualizer = GOoDA.Visualizer.getInstance();
    
    if(!report)
      report = new GOoDA.Report({name: name, parentContainer: visualizer, onLoadHandler: onLoadHandler});
    else{
      report.highlight();
      onLoadHandler && onLoadHandler(report);
    }

    return report;
  }
});
