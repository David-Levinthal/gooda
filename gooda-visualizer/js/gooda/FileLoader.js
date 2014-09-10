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

require(["dojo/_base/declare"], function(declare){
  declare("GOoDA.FileLoader", null, {
  
    HEADER_ROW: 0,
    CHILDREN_ROW: 1,
    PERIOD_ROW: 3,
    MULTIPLEX_ROW: 4,
    PENALTY_ROW: 5,
    CYCLE_ROW: 6,
    FIRST_DATA_ROW: 7,
  
    constructor: function(params){
    },
    
    getHotProcessData: function(request){
      var self = this;
      var parent = null;
      var curProc = 0;

      this.readGrid(request.report + '/spreadsheets/process.csv', function(data){
        request.success(self.convert(1, 3, data, undefined, function(iRow, oRow, convertedData){
          var proc = iRow[1];

          if(proc !== undefined){
            parent = oRow;
            curProc = proc;
          }else 
            oRow['parent'] = parent;
        }));
      }, request.failure);
    },
    
    getHotFunctionData: function(request){
      var self = this;
      var curFunID = -1;

      this.readGrid(request.report + '/spreadsheets/function_hotspots.csv', function(data){
        request.success(self.convert(3, 8, data, undefined, function(iRow, oRow, convertedData){
          var funID = iRow[1];
          
          if(funID != curFunID){
            parent = oRow;
            curFunID = funID;
            oRow['FunctionID'] = funID;
          }else{
            oRow['parent'] = parent;
            oRow['FunctionID'] = iRow[2];
          }
        }));
      }, request.failure);
    },

    getASMData: function(request){
      var self = this;
      var parent = null;
      var curBB = 0;

      this.readGrid(request.report + '/spreadsheets/asm/' + request.functionID + '_asm.csv', function(data){
        request.success(self.convert(2, 8, data, function(convertedData){
          convertedData.sourceLines = {};
          convertedData.functionName = convertedData.columns[5].summary;
        }, function(iRow, oRow, convertedData){
          var bb = iRow[1];

          if(bb != curBB){
            parent = oRow;
            curBB = bb;
            convertedData.sourceLines[iRow[3]] = true;
          }else 
            oRow['parent'] = parent;
        }));
      }, request.failure);
    },

    getSRCData: function(request){
      var self = this;

      this.readGrid(request.report + '/spreadsheets/src/' + request.functionID + '_src.csv', function(data){
        request.success(self.convert(1, 3, data));
      }, request.failure);
    },

    getCFGData: function(request){    
      this.readFile(request.report + '/spreadsheets/cfg/' + request.functionID + '_cfg.svg', function(data){
        request.success($(data.substring(data.indexOf('<svg'))).last());
      }, request.failure);
    },
    
    getCGData: function(request){
      this.readFile(request.report + '/spreadsheets/cg/' + request.processID + '_cg.svg', function(data){
        request.success($(data.substring(data.indexOf('<svg'))).last());
      }, request.failure);
    },

    convert: function(firstColumn, firstEventColumn, data, initialize, exec, finalize){
      var lastEventColumn = data[0].length - 1;
      var convertedData = this.createConversionData(firstColumn, firstEventColumn, lastEventColumn, data, initialize);

      this.convertGrid(firstColumn, firstEventColumn, lastEventColumn, data, convertedData, exec);
      this.updateSummary(firstEventColumn - firstColumn, convertedData);
      if(finalize) finalize(convertedData);

      return convertedData;
    },

    createConversionData: function(firstColumn, firstEventColumn, lastEventColumn, data, initialize){
      var convertedData = {
        grid: [],
        columns: [],
        referencePeriod: data[this.PERIOD_ROW][firstEventColumn],
        inverseReferencePeriod: 1/data[this.PERIOD_ROW][firstEventColumn],
        referenceCycles: data[data.length - 1][firstEventColumn]*data[this.MULTIPLEX_ROW][firstEventColumn]
      };

      for(var i = firstColumn; i <= lastEventColumn; i++){
        convertedData.columns[i - firstColumn] = {
          index: i - firstColumn,
          name: data[this.HEADER_ROW][i],
          id: data[this.HEADER_ROW][i],
          field: data[this.HEADER_ROW][i],
          summary: data[data.length - 1][i],
          period: data[this.PERIOD_ROW][i],
          children: data[this.CHILDREN_ROW][i],
          multiplex: data[this.MULTIPLEX_ROW][i],
          penalty: data[this.PENALTY_ROW][i],
          isCycle: data[this.CYCLE_ROW][i],
          isEvent: i >= firstEventColumn,
          isHybrid: true,
          maxPercentageLength: 0,
          expanded: false,
          visible: true
        };
      }

      // Determine if it's a diff spreadsheet
      if(data[data.length - 2][1] == "Global sample breakdown"){
        for(var i = firstColumn; i <= lastEventColumn; i++){
          convertedData.columns[i - firstColumn].summaryBase = data[data.length - 2][i];
          convertedData.columns[i - firstColumn].summaryDiff = data[data.length - 3][i];
        }

        convertedData.isDiff = true;
      }

      if(initialize) initialize(convertedData);
      this.createTree(convertedData.columns);
      this.createSummary(convertedData.columns);
      return convertedData;
    },

    createTree: function(columns){
      for(var i = 0; i < columns.length; i++){
        var column = columns[i];

        if(column.isCycle) column.isHybrid = false;
        column.isTop = true;

        i += this.createTree1(columns, i, '');
      }
    },

    createTree1: function(columns, index, prefix){
      var displ = 0;
      var column = columns[index];
      var n = parseInt(column.children.split(':')[1]);

      column.children = [];
      column.name = prefix + column.name;

      for(var i = 1; i <= n; i++){
        var child = index + displ + i;

        column.children.push(columns[child]);
        displ += this.createTree1(columns, child, prefix + '►');
      }

      if(column.children.length) column.isHybrid = false;

      return n + displ;
    },

    createSummary: function(columns){     
      for(var i = 0; i < columns.length; i++){
        var column = columns[i];

        if(column.isEvent){
          column.summary = Math.round(column.summary*column.multiplex);

          if(column.summaryDiff){
            column.summaryDiff = Math.round(column.summaryDiff*column.multiplex);
            column.summaryBase = Math.round(column.summaryBase*column.multiplex);
          }
        }else{
          column.summary = 0;

          if(column.summaryDiff){
            column.summaryDiff = 0;
            column.summaryBase = 0;
          }
        }
      }
    },

    convertGrid: function(fC, fEC, lEC, data, cData, custom){
      var maxRow = null;
      var maxCycles = 0;
      var len = cData.isDiff ? data.length - 3 : data.length - 1;

      for(var i = this.FIRST_DATA_ROW; i < len; i++){
        var cycles = 0;
        var iRow = data[i];
        var oRow = (cData.grid[i - this.FIRST_DATA_ROW] = {id : i - this.FIRST_DATA_ROW});
        var irCycles = 100/iRow[fEC];

        if(custom) custom(iRow, oRow, cData);

        for(var j = fC; j < fEC; j++){
          var column = cData.columns[j - fC];
          oRow[column.field] = iRow[j];
          column.summary = this.getMaxLength(column.summary, iRow[j]);
        }

        for(; j <= lEC; j++){
          column = cData.columns[j - fC];
          if(column.isTop) cycles += iRow[j];
          if(column.summary) oRow[column.field] = Math.round(iRow[j] * column.multiplex);

          if(iRow[fEC]){
            var percentage = oRow[column.field]*column.penalty*column.period*cData.inverseReferencePeriod*irCycles;
            if(percentage > column.maxPercentageLength) column.maxPercentageLength = Math.floor(percentage);
          }
        }

        if(!oRow.parent && cycles >= maxCycles){
          maxCycles = cycles;
          maxRow = oRow;
        }
      }

      for(var i = 0; i < cData.columns.length; i++)
        cData.columns[i].maxPercentageLength = cData.columns[i].maxPercentageLength.toString().length;

      cData.maxRow = maxRow;
    },

    getMaxLength: function(summaryLength, value){
      value = value || 0;
      return Math.max(summaryLength, value.length ? value.length : value);
    },

    updateSummary: function(firstEventColumn, convertedData){  
      for(var i = 0; i < firstEventColumn; i++){
        column = convertedData.columns[i];

        switch(column.field){
          case GOoDA.Columns.FUNCTIONNAME:
            if(convertedData.isDiff){
              column.summary = this.getBlankString(column.summary + 3, "reference");
              column.summaryDiff = "diff";
              column.summaryBase = "new";
            }else
              column.summary = this.getBlankString(column.summary + 3, '');

            break;
            
          case GOoDA.Columns.PROCESSPATH:
            if(convertedData.isDiff){
              column.summary = this.getBlankString(column.summary + 3, "reference");
              column.summaryDiff = "diff";
              column.summaryBase = "new";
            }else
              column.summary = this.getBlankString(column.summary + 3, '');

            break;

          case GOoDA.Columns.ADDRESS:
          case GOoDA.Columns.OFFSET:
          case GOoDA.Columns.LENGTH:
            column.summary = this.getBlankString(column.summary);
            break;

          case GOoDA.Columns.LINENUMBER:
            column.summary = this.getBlankString(column.summary, '', true);
            break;

          case GOoDA.Columns.SOURCE:
          case GOoDA.Columns.DISASSEMBLY:
          case GOoDA.Columns.MODULE:
          case GOoDA.Columns.PROCESS:
          case GOoDA.Columns.PRINCIPALFILE:
          case GOoDA.Columns.INITIALFILE:
          case GOoDA.Columns.MODULEPATH:
            column.summary = this.getBlankString(column.summary);
            break;

          case GOoDA.Columns.PRINCIPALLINENUMBER:
          case GOoDA.Columns.INITIALLINENUMBER:
            column.summary = this.getBlankString(column.summary, true);
            break;

          default:
            break;
        }
      }
    },

    getBlankString: function(summaryLength, prefix, isNum){   
      prefix = prefix || '';

      isNum && (summaryLength = String(summaryLength).length);

      if(summaryLength > prefix.length){
        summaryLength = summaryLength - prefix.length;
        return prefix + new Array(summaryLength + 1).join(' ');
      }else
        return prefix;
    },

    readFile: function(file, success, error){
      if(file.indexOf("reports/") != 0){
        var response;

        try{
          response = jsFs.read(file);
        }catch(e){
          response = null;
        }

        if(response){
          success(response);
        }else{
          error();
        }

        return;
      }

      $.ajax({
        url: file,
        cache: false,
        dataType: 'text',
        success: success,
        error: error
      });
    },

    readGrid: function(file, success, error){
      if(file.indexOf("reports/") != 0){
        var response = null;

        try{
          response = jsFs.read(file);
        }catch(e){
          response = null;
        }
       
        if(response){
          success(eval('(' + response + ')'));
        }else{
          error();
        }

        return;
      }

      $.ajax({
        url: file,
        cache: false,
        dataType: 'text',
        success: function(response, status, xhr){
            success(eval('(' + response + ')'));
        },
        error: error
      });
    }
  });
});
