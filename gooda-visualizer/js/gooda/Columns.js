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
  declare("GOoDA.Columns", null, {
  });
  
  GOoDA.Columns.FUNCTIONNAME = 'Function Name';
  GOoDA.Columns.SOURCE = 'Source';
  GOoDA.Columns.DISASSEMBLY = 'Disassembly';
  GOoDA.Columns.OFFSET = 'Offset';
  GOoDA.Columns.LENGTH = 'Length';
  GOoDA.Columns.ADDRESS = 'Address';
  GOoDA.Columns.PRINCIPALLINENUMBER = 'Princ_L#';
  GOoDA.Columns.INITIALLINENUMBER = 'Init_L#';
  GOoDA.Columns.LINENUMBER = 'Line Number';
  GOoDA.Columns.MODULE = 'Module';
  GOoDA.Columns.PROCESS = 'Process';
  GOoDA.Columns.PRINCIPALFILE = 'Principal File';
  GOoDA.Columns.INITIALFILE = 'Initial File';
  GOoDA.Columns.PROCESSPATH = 'Process Path';
  GOoDA.Columns.MODULEPATH = 'Module Path';
  GOoDA.Columns.FUNCTIONID = 'FunctionID';
  
  GOoDA.Columns.UNHALTEDCYCLES = 'unhalted_core_cycles';
  GOoDA.Columns.STALLCYCLES = 'uops_retired:stall_cycles';
});