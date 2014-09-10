/*
  Visualizer for the Generic Optimization Data Analyzer, Copyright (c)
  2012, The Regents of the University of California, through Lawrence
e Berkeley National Laboratory (subject to receipt of any required
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
         "dijit/Toolbar",
         "dijit/form/Button",
         "dijit/form/TextBox",
         "dijit/layout/ContentPane"], function(declare,
                                               Toolbar,
                                               Button,
                                               TextBox,
                                               ContentPane){
  declare("GOoDA.EventTable", null, {
    constructor : function(params){
      declare.safeMixin(this, params);

      var self = this;
      var sortcol = "";
      var sortdir = false;
      var sortDiff = null;
      var searchString = "";
      var sampleMode = true;
      var cycleMode = true;
      var dataView = null;
      var data = []
      var grid = null;
      var collapsed = false;
      var selectedRow = undefined;
      var selectedRows = [];
      var criterias = null;

      var referencePeriod = this.data.referencePeriod;
      var inverseReferencePeriod = this.data.inverseReferencePeriod;

      function numberFormatter(row, cell, value, column, dataContext){
        if(!value) return '';

        if(column.isCycle){
          var referenceCycles;
          var cvalue;

          if(self.data.isDiff && dataContext._collapsed != undefined){
            referenceCycles = self.data.grid[dataContext.id + 2][column.id];
          }else{
            if(column.name == GOoDA.Columns.UNHALTEDCYCLES)
              referenceCycles = column.summary;
            else
              referenceCycles = row >= 0 ? dataContext['unhalted_core_cycles'] : self.data.referenceCycles;
          }

          if(sampleMode && cycleMode){
            cvalue = Math.floor(value*column.period*column.penalty*inverseReferencePeriod);

            if(referenceCycles)
              cvalue = buildField(column, cvalue, Math.floor(100*cvalue/referenceCycles));
            else
              cvalue = buildField(column, cvalue);
          }
          else if(sampleMode && !cycleMode){
            cvalue = value;

            if(!column.isHybrid && referenceCycles) 
              cvalue = buildField(column, cvalue, Math.floor(100*cvalue/referenceCycles));
            else if(!column.isHybrid)
              cvalue = buildField(column, cvalue);
          }
          else if(cycleMode){
            cvalue = (value*column.period*column.penalty).toExponential(3);

            if(referenceCycles)
              cvalue = buildField(column, cvalue, Math.floor(100*cvalue/(referenceCycles*referencePeriod)));
            else
              cvalue = buildField(column, cvalue);
          }else{
            cvalue = (value*column.period).toExponential(3);

            if(!column.isHybrid && referenceCycles) 
              cvalue = buildField(column, cvalue, Math.floor(100*cvalue/(referenceCycles*referencePeriod)));
            else if(!column.isHybrid)
              cvalue = buildField(column, cvalue);
          }

          return value < 0 ? cvalue : " " + cvalue;
        }else if(sampleMode){
          return value < 0 ? value : " " + value;
        }else{
          var ret = (value*column.period).toExponential(3);
          return value < 0 ? ret : " " + ret;
        }
      };
      
      function sourceFormatter(row, cell, value, column, dataContext){
        return $('<div/>').text(value).html();
      };
      
      function treeFormatter(row, cell, value, column, dataContext) {     
        if(row == -1) return value;
        if(value === undefined) return '';

        if (dataContext.parent === undefined) {
          if (dataContext._collapsed)
            return "<span class='toggle expand'></span>" + value;
          else
            return "<span class='toggle collapse'></span>" + value;
        }
        else
          return "<span class='toggle'></span>" + value;
      };
      
      function buildView(){
        var toolbar = new Toolbar({
          region: 'top',
          style: "padding: 0"
        });

        var restoreButton = new Button({
          label: 'Restore Order',
          showLabel: false,
          iconClass: "dijitEditorIcon dijitEditorIconIconPaste",
          onClick: function(){
            self.restoreOrder();
          }
        });

        var expandButton = new Button({
          label: 'Toggle Expansion',
          showLabel: false,
          iconClass: "dijitEditorIcon dijitEditorIconListBulletOutdent",
          onClick: function(){
            self.toggleExpansion();
          }
        });

        var unhideButton = new Button({
          label: 'Unhide Columns',
          showLabel: false,
          iconClass: "dijitEditorIcon dijitEditorIconUndo",
          onClick: function(){
            self.report.notifyViews({id: 'unhideColumns'});
          }
        });

        var cyclesButton = new Button({
          title: 'Cycles',
          label: 'Cycles',
          onClick: function(){
            toggleCycleMode(this);
          }
        });

        var samplesButton = new Button({
          title: 'Samples',
          label: 'Samples',
          onClick: function(){
            toggleSampleMode(this);
          }
        });

        var searchBox = new TextBox({
          placeHolder: 'Enter search term',
          style: 'float: right; margin: 2px',
          onKeyUp: function(){
            searchString = this.get('value');
            dataView.refresh();
            restoreSelectedRows();
          }
        });

        toolbar.addChild(restoreButton);
        if(self.expand)
          toolbar.addChild(expandButton);
        toolbar.addChild(unhideButton);
        toolbar.addChild(cyclesButton);
        toolbar.addChild(samplesButton);
        toolbar.addChild(searchBox);

        var pane = new ContentPane({
          region: 'center',
          style: "padding: 0; margin-top: -6px"
        });

        self.container.addChild(toolbar);
        self.container.addChild(pane);
        self.container = pane.domNode;
        self.columns = [];
        self.options = {
          rowCssClasses: self.rowCssClasses,
          syncColumnCellResize: true,
          enableCellNavigation: false,
          enableColumnReorder: false,
          hideColumnHandler: self.hideColumnHandler,
          isDiff: self.data.isDiff
        };
      }
      
      function buildField(column, cycles, percentage){
        var spacerLen = 0;
        percentage = percentage !== undefined ? " (" + percentage + "%)" : "";
        spacerLen = column.maxPercentageLength + 5 - percentage.length;
        spacerLen = spacerLen > 0 ? spacerLen : 0;
        return cycles.toString() + new Array(spacerLen).join(' ') + percentage;
      }

      function comparer(a,b){
        var first = a.parent === undefined ? a : a.parent;
        var second = b.parent === undefined ? b : b.parent;
        var x = first[sortcol], y = second[sortcol];
        var diff = sortDiff(x, y);
        
        if(diff){ // number
          return (diff > 0 ? -1 : 1);
        }else{ // string
          return sortdir ? (a.id > b.id ? 1 : -1) : (a.id > b.id ? -1 : 1);
        }
      }
      
      function hexDiff(a, b){
        var diff = a.length - b.length;
        return diff ? diff : ((a == b) ? 0 : ((a > b) ? 1 : -1))
      }
      
      function defaultDiff(a, b){
        return a - b;
      }

      function filter(item) {
        if(criterias)
          for(var i in criterias)
            if(criterias[i] && item[i] != criterias[i]) 
              return false;

        if (searchString != "" && item[self.source] && item[self.source].indexOf(searchString) == -1)
          return false;

        if (item.parent !== undefined)
          if (item.parent._collapsed)
            return false;

        return true;
      }

      function restoreSelectedRows(noscroll){
        var indices = []

        for(var i = 0; i < selectedRows.length; i++)
          indices.push(dataView.getRowById(selectedRows[i].id));

        grid.setSelectedRows(indices);
        if(!noscroll && indices.length) grid.scrollToRow(indices[0]);
      }

      function toggleCycleMode(button){
        cycleMode = !cycleMode;
        if(cycleMode){
          button.set('label', 'Cycles');
          button.set('title', 'Cycles');

          for(var i = 0; i < self.columns.length; i++){
            if(self.columns[i].cssClass == "eventLeaf" 
              && self.columns[i].name != GOoDA.Columns.UNHALTEDCYCLES
              && self.columns[i].name != GOoDA.Columns.STALLCYCLES){
              self.columns[i].cssClass = "cycleLeaf";
              self.columns[i].secondaryCssClass = "cycleLeaf";
            }
          }
        }
        else{
          button.set('label', 'Events');
          button.set('title', 'Events');

          for(var i = 0; i < self.columns.length; i++){
            if(self.columns[i].cssClass == "cycleLeaf" 
              && self.columns[i].name != GOoDA.Columns.UNHALTEDCYCLES
              && self.columns[i].name != GOoDA.Columns.STALLCYCLES){
              self.columns[i].cssClass = "eventLeaf";
              self.columns[i].secondaryCssClass = "eventLeaf";
            }
          }
        }

        grid.minimizeWidth();
        grid.invalidate();
      }

      function toggleSampleMode(button){
        sampleMode = !sampleMode;
        if(sampleMode){
          button.set('label', 'Samples');
          button.set('title', 'Samples');
        }else{
          button.set('label', 'Totals');
          button.set('title', 'Totals');
        }
        grid.minimizeWidth();
        grid.invalidate();
      }

      function cycleMode(){
        eventMode = false;
        cycleMode = true;
        grid.minimizeWidth();
        grid.invalidate();
      } 

      function eventMode(){
        eventMode = true;
        cycleMode = false;
        grid.minimizeWidth();
        grid.invalidate();
      }

      function wireEvents(){      
        grid.onClick = function(e, row, cell){
          var target = $(e.target);

          if (target.hasClass('toggle')) {
            var item = dataView.rows[row];

            if (item) {
              if (!item._collapsed)
              item._collapsed = true;
              else
              item._collapsed = false;

              dataView.updateItem(item.id, item);
              restoreSelectedRows(true);
            }

            return true;
          }else if(self.codeHandler){
            self.codeHandler(target, e, row, cell, dataView.getRow(row), self);
            return true;
          }

          return false;
        }
        
        grid.onDblClick = function(e, row, cell){
          var target = $(e.target);
          
          if(self.altCodeHandler){
            self.altCodeHandler(target, e, row, cell, dataView.getRow(row), self);
            return true;
          }

          return false;
        }

        grid.onSort = function(sortCol, sortAsc) {
          sortcol = sortCol.id;
          sortdir = sortAsc;

          switch(sortCol.id){
            case GOoDA.Columns.OFFSET:
            case GOoDA.Columns.LENGTH:
            case GOoDA.Columns.ADDRESS:
              sortDiff = hexDiff;
              break;
              
            default:
              sortDiff = defaultDiff;
              break;
          }
          
          dataView.sort(comparer,sortAsc);
          restoreSelectedRows();
        };

        dataView.onRowCountChanged.subscribe(function(args) {
          grid.updateRowCount();
          grid.render();
        });

        dataView.onRowsChanged.subscribe(function(rows) {
          grid.removeRows(rows);
          grid.render();
        });

        if(!Modernizr.touch)
          $('.slick-row').live({
            mouseenter: function(){
              if(!$(this).hasClass('selected'))
                $(this).addClass('hover');
            },

            mouseleave: function(){
              $(this).removeClass('hover');
            }
          });
      }

      function addColumns(){
        for(var i = 0; i < self.data.columns.length; i++)
          addEventColumn(self.data.columns[i]);
      }

      function addEventColumn(column){
        $.extend(column, {flex: true, sortable: true});

        if(column.name == GOoDA.Columns.UNHALTEDCYCLES)
          column.cssClass = column.secondaryCssClass = "cycleLeafMain";
        else if(column.isEvent && column.isCycle && column.children.length)
          column.cssClass = column.secondaryCssClass = "cycleBranch";
        else if(column.isEvent && column.isCycle)
          column.cssClass = column.secondaryCssClass = "cycleLeaf";

        switch(column.id){
          case GOoDA.Columns.SOURCE:
            column.sortable = false;
            column.cssClass = 'code';
            column.formatter = sourceFormatter;
            column.resizable = true;
            break;

          case GOoDA.Columns.DISASSEMBLY:
            column.sortable = false;
            column.cssClass = 'code';
            column.formatter = treeFormatter;
            column.resizable = true;
            break;

          case GOoDA.Columns.FUNCTIONNAME:
          case GOoDA.Columns.PROCESSPATH:
            column.cssClass = 'code';
            column.formatter = treeFormatter;
            column.resizable = true;
            break;

          case GOoDA.Columns.OFFSET:
          case GOoDA.Columns.LENGTH:
          case GOoDA.Columns.ADDRESS:
          case GOoDA.Columns.PRINCIPALLINENUMBER:
          case GOoDA.Columns.INITIALLINENUMBER:
          case GOoDA.Columns.LINENUMBER:
            break;

          case GOoDA.Columns.MODULE:
          case GOoDA.Columns.PROCESS:
          case GOoDA.Columns.PRINCIPALFILE:
          case GOoDA.Columns.INITIALFILE:
          case GOoDA.Columns.MODULEPATH:
            column.cssClass = 'names'
            column.resizable = true;
            break;

          default:
            column.formatter = numberFormatter;
            break;
        }

        self.columns.push(column);
      }

      function buildTable(){
        $('<div class="grid" style="width:100%;height:100%;"></div>').appendTo(self.container);
        dataView = new Slick.Data.DataView();
        grid = new Slick.Grid($('#' + self.container.getAttribute('widgetid') + ' .grid'), dataView.rows, self.columns, self.options);
        wireEvents();
      }

      function fillTable(){
        dataView.beginUpdate();
        dataView.setItems(self.data.grid.slice(0));
        dataView.setFilter(filter);
        dataView.endUpdate();
      }

      this.restoreOrder = function(){
        sortcol = 'id';
        sortDiff = defaultDiff;

        grid.clearSorting();
        dataView.sort(comparer, false);
        restoreSelectedRows();
      }

      this.toggleExpansion = function(){
        collapsed = !collapsed;

        for(var i = 0; i < dataView.rows.length; i++){
          var item = dataView.rows[i];
          if(item.parent === undefined) 
            item._collapsed = collapsed;
        }

        grid.scrollToRow(0);
        grid.invalidate();
        dataView.refresh();
        restoreSelectedRows();
      }

      this.unhideColumns = function(){
        for(var i = 0; i < this.columns.length; i++){
          this.columns[i].visible = true;
        }

        grid.setColumns(this.columns);
      }

      this.hideColumn = function(name){
        for(var i = 0; i < this.columns.length; i++){
          var column = this.columns[i];

          if(column.name == name){
            if(column.visible){
              column.visible = false;
              grid.setColumns(this.columns);
            }

            break;
          }
        }
      }

      this.unselect = function(){
        this.selectRows([]);
      }

      this.refresh = function(){
        grid.restoreViewport();
        grid.render();
      }

      this.filter = function(crit){
        criterias = crit;
        dataView.setFilter(filter);
      }

      this.select = function(selector, multiple){
        var selection = [];

        for(var i = 0; i < dataView.rows.length; i++){
          if(selector(dataView.rows[i])){
            if(!multiple){
              this.selectRows([dataView.rows[i]]);
              return dataView.rows[i];
            }
            else
              selection.push(dataView.rows[i]);
          }
        }

        if(selection.length){
          this.selectRows(selection);
          return selection;
        }
        return null;
      }

      this.selectRows = function(rows){
        selectedRows = rows;
        restoreSelectedRows();
      }

      this.eachRow = function(fun){
        for(var i = 0; i < dataView.rows.length; i++)
          fun(dataView.rows[i]);

        grid.invalidate();
      }
      
      this.resize = function(){
        grid && grid.resizeCanvas();
      }
      
      this.getRow = function(id){
        return dataView.getRowById(id);
      }

      buildView();
      addColumns();
      buildTable();
      fillTable();
    }
  })
});
