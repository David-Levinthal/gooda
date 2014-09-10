#!/usr/bin/python
"""Copyright 2012 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License
"""

"""Generate difference spreadsheet of 2 Gooda function_hotspot.csv
or process.csv files.
"""

__author__ = 'tejohnson@google.com (Teresa Johnson)'

import sys
import re
import copy

class Error(Exception):
    pass

class Spreadsheet(object):
  __slots__ = ('header_dict','header_order','entriesL1_dict',
               'entriesL2_dict','type_info')
  def __init__(self):
    self.header_dict = {}
    self.header_order = []
    self.entriesL1_dict = {}
    self.entriesL2_dict = {}
    self.type_info = None

  def HeaderKey(self, list):
    return self.type_info.HeaderKey(list)

  def EntryKeyL1(self, list):
    return self.type_info.EntryKeyL1(list)

  def EntryKeyL2(self, list):
    return self.type_info.EntryKeyL2(list)

  def GSKey(self, list):
    return self.type_info.GSKey(list)

  def FirstCol(self):
    return self.type_info.first_data_index

  def LastCol(self):
    return self.type_info.last_data_index

class SpreadsheetTypeInfo(object):
  __slots__ = ('header_key_index','global_sample_key_index','first_data_index',
               'last_data_index','func_index','process_index','module_index')
  def __init__(self, header_list):
    if header_list[3] == '"Function Name"':
      self.header_key_index = 3
      self.first_data_index = 8
      self.func_index = 3
      self.process_index = 7
      self.module_index = 6
      if header_list[self.module_index] != '"Module"':
        raise Error('Did not find "Module" at index %d in %s' % (self.module_index,header_list))
      if header_list[self.process_index] != '"Process"':
        raise Error('Did not find "Process" at index %d in %s' % (self.process_index,header_list))
    elif header_list[1] == '"Process Path"':
      self.header_key_index = 2
      self.first_data_index = 3
      self.func_index = None
      self.process_index = 1
      self.module_index = 2
      if header_list[self.module_index] != '"Module Path"':
        raise Error('Did not find "Module Path" at index %d in %s' % (self.module_index,header_list))
    else:
      raise Error('Unexpected header line format: %s' % header_list)
    self.global_sample_key_index = 1
    # Last column is empty
    self.last_data_index = len(header_list) - 2

  def HeaderKey(self, list):
    return list[self.header_key_index]

  def EntryKeyL1(self, list):
    if self.func_index:
      # Concatenate function, module and process
      return list[self.func_index] + list[self.module_index] + list[self.process_index]
    else:
      return list[self.process_index]

  def EntryKeyL2(self, list):
    if self.func_index:
      return None
    else:
      return list[self.module_index]

  def GSKey(self, list):
    return list[self.global_sample_key_index]

def CheckMultiplexFormat(list, start_col, stop_col):
  if ((list[start_col-1] != '' and
       list[start_col-1] != '"Multiplex"') or
      list[start_col] < 1 or
      list[stop_col+1] != ''):
    raise Error('Unexpected multiplex line format: %s', list)

def BuildSpreadsheet(filename):
  input_file = open(filename, 'r')
  found_header = False
  found_cycles = False
  prev_L1_key = None
  ss = Spreadsheet()
  for line in input_file:
    match = re.search(r'^\[$|^]$|^$', line)
    if match:
      continue

    list = re.findall(r'"[^, ][^"]*"|[^\[\]," \n]+|(?<=[\[,])(?=[\],])|(?<=[\[,] )(?=[\],])', line)
    if list[0] != '':
      raise Error("Expected first column to be empty: %s" % line)
   #print list

    if not found_header:
      ss.type_info = SpreadsheetTypeInfo(list)
      found_header = True

    if not found_cycles:
      key = ss.HeaderKey(list)
      #print key
      if key == '"Cycles"':
        found_cycles = True
      if key == '"Multiplex"':
        CheckMultiplexFormat(list, ss.FirstCol(), ss.LastCol())
      ss.header_dict[key] = list
      ss.header_order.append(key)
      continue

    if ss.GSKey(list) == '"Global sample breakdown"':
      ss.entriesL1_dict[ss.GSKey(list)] = list
    elif ss.EntryKeyL1(list):
      ss.entriesL1_dict[ss.EntryKeyL1(list)] = list
      prev_L1_key = ss.EntryKeyL1(list)
    else:
      if not prev_L1_key:
        raise Error("Saw level 2 key before level 1 key: %s" % line)
      if prev_L1_key not in ss.entriesL2_dict:
        ss.entriesL2_dict[prev_L1_key] = {}
      ss.entriesL2_dict[prev_L1_key][ss.EntryKeyL2(list)] = list
      #print prev_L1_key, ":", ss.EntryKeyL2(list), ":", list

  input_file.close()
  return ss

def PrintCsvLine(list, last=False):
  printline = '[' + ', '.join(list) + ']'
  if not last:
    printline = printline + ","
  print printline

def AddMissingEntries(dictA, dictB, dictA2, dictB2,
                      start_col, stop_col):
  for (k, v) in dictA.iteritems():
    if k not in dictB:
      dictB[k] = copy.deepcopy(dictA[k])
      for i in range(start_col, stop_col + 1):
        dictB[k][i] = 0
    if dictA2 and k in dictA2:
      if k not in dictB2:
        dictB2[k] = {}
      AddMissingEntries(dictA2[k], dictB2[k], None, None, start_col, stop_col)

def ConvertAndScale(dict, dict2, scale_array,
                    start_col, stop_col):
  scale = 1.0
  for (k, v) in dict.iteritems():
    for i in range(start_col, stop_col + 1):
      if scale_array:
        scale = scale_array[i]
      dict[k][i] = scale*float(dict[k][i])
    if dict2 and k in dict2:
      ConvertAndScale(dict2[k], None, scale_array, start_col, stop_col)

def ComputeChange(dictA, dictB, dictA2, dictB2,
                  change_dict, change_dict2,
                  start_col, stop_col):
  for (k, v) in dictA.iteritems():
    change_dict[k] = copy.deepcopy(dictB[k])
    for i in range(start_col, stop_col + 1):
      change_dict[k][i] = str(int(change_dict[k][i] + dictA[k][i]))
    if dictA2 and k in dictA2:
      change_dict2[k] = {}
      ComputeChange(dictA2[k], dictB2[k], None, None,
                    change_dict2[k], None, start_col, stop_col)

def CompareDicts(ref_ss, new_ss):

  for (k, v) in ref_ss.header_dict.iteritems():
    if k != '"Multiplex"' and v != new_ss.header_dict[k]:
      raise Error('Header lines not equal: "%s" != "%s"' % (v, new_ss.header_dict[k]))

  start_col = ref_ss.FirstCol()
  stop_col = ref_ss.LastCol()
  ref_multiplex = ref_ss.header_dict['"Multiplex"']
  new_multiplex = new_ss.header_dict['"Multiplex"']
  multiplex_ratio = copy.deepcopy(new_multiplex)
  for i in range(start_col, stop_col + 1):
    multiplex_ratio[i] = float(multiplex_ratio[i])/float(ref_multiplex[i])
  new_multiplex = ref_multiplex
  #print multiplex_ratio

  # First add entries found in one to the other if not there
  AddMissingEntries(ref_ss.entriesL1_dict, new_ss.entriesL1_dict,
                    ref_ss.entriesL2_dict, new_ss.entriesL2_dict,
                    start_col, stop_col)
  AddMissingEntries(new_ss.entriesL1_dict, ref_ss.entriesL1_dict,
                    new_ss.entriesL2_dict, ref_ss.entriesL2_dict,
                    start_col, stop_col)
  ConvertAndScale(ref_ss.entriesL1_dict, ref_ss.entriesL2_dict, None,
                  start_col, stop_col)
  ConvertAndScale(new_ss.entriesL1_dict, new_ss.entriesL2_dict, multiplex_ratio,
                  start_col, stop_col)

  change_dictL1 = {}
  change_dictL2 = {}
  ComputeChange(ref_ss.entriesL1_dict, new_ss.entriesL1_dict,
                ref_ss.entriesL2_dict, new_ss.entriesL2_dict,
                change_dictL1, change_dictL2,
                start_col, stop_col)

  print '['
  for h in ref_ss.header_order:
    PrintCsvLine(ref_ss.header_dict[h])

  globalsample = change_dictL1['\"Global sample breakdown\"']
  del change_dictL1['\"Global sample breakdown\"']

  for key, value in sorted(change_dictL1.iteritems(), key=lambda (k,v): abs(int(v[start_col])), reverse=True):
    PrintCsvLine(value)
    if key in change_dictL2:
      for key2, value2 in sorted(change_dictL2[key].iteritems(), key=lambda (k,v): abs(int(v[start_col])), reverse=True):
        PrintCsvLine(value2)

  PrintCsvLine(globalsample, True)
  print ']'


def main():
  if len(sys.argv) != 3:
    print 'usage: gooda_diff.py file_ref file_new'
    sys.exit(1)

  filename_ref = sys.argv[1]
  filename_new = sys.argv[2]

  ref_ss = BuildSpreadsheet(filename_ref)
  new_ss = BuildSpreadsheet(filename_new)
  CompareDicts(ref_ss, new_ss)
  sys.exit(0)

if __name__ == '__main__':
  main()
