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
         "dijit/Toolbar",
         "dijit/form/Button"], function(declare, 
                                        ContentPane,
                                        Toolbar,
                                        Button){
  declare("GOoDA.GraphPane", null, {
    constructor: function(params){
      declare.safeMixin(this, params);
      
      var self = this;
      
      var toolbar = new Toolbar({
        region: 'top',
        style: "padding: 0; height: 25px",
      });
      
      var zoomIn = new Button({
        label: 'Zoom In',
        showLabel: false,
        iconClass: "visualizerIcon visualizerIconMagnifyIn",
        onClick: function(){
          self.svg.SVGScrollView('zoom', 1.5);
        }
      });
      
      var zoomOut = new Button({
         label: 'Zoom Out',
         showLabel: false,
         iconClass: "visualizerIcon visualizerIconMagnifyOut",
         onClick: function(){
           self.svg.SVGScrollView('zoom', 0.67);
         }
       });
      
      this.gPane = new ContentPane({
        region: 'center',
        style: "padding: 0; margin-top: -6px; overflow: hidden"
      });
      
      toolbar.addChild(zoomIn);
      toolbar.addChild(zoomOut);
      this.container.addChild(toolbar);
      this.container.addChild(this.gPane);

      $(this.gPane.domNode).append(this.svg);
      this.svg.SVGScrollView({
        clickHandler: this.clickHandler,
        altClickHandler: this.altClickHandler
      });
    },
    
    resize: function(width, height){
      this.svg && this.svg.SVGScrollView('resize', width, height);
    },
    
    select: function(id, multi){    
      this.svg && this.svg.SVGScrollView('select', id, multi);
    },
    
    unselect: function(){
      this.svg && this.svg.SVGScrollView('select', -10);
    },
    
    reload: function(svg, selection){
      $(this.gPane.domNode).empty();
      
      this.svg = svg;
      $(this.gPane.domNode).append(this.svg);
      
      this.svg.SVGScrollView({
        clickHandler: this.clickHandler,
        selection: selection
      });
    },
    
    empty: function(){
      $(this.gPane.domNode).empty();
      this.svg = null;
    }
  })
});