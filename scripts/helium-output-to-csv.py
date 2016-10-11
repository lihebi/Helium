#!/usr/bin/env python3
# parse the output file from HELIUM

# HELIUM_INPUT_SPEC
# int_iname.strlen = 151
# addr_iname = 0x61300000de80
# isnull_iname = 0
# HELIUM_INPUT_SPEC_END
# HELIUM_POI_INSTRUMENT
# int_ifname.size = 1024
# addr_ifname = 0x6059e0
# int_iname.strlen = 151
# addr_iname = 0x61300000de80
# isnull_iname = 0
# HELIUM_POI_INSTRUMENT_END
# HELIUM_AFTER_POI
# HELIUM_TEST_SUCCESS

# The table looks like:

# |       | input  |        |        | output  |         |         | code |
# |-------+--------+--------+--------+---------+---------+---------+------|
# |       | input1 | input2 | input3 | output1 | output2 | output3 |      |
# |-------+--------+--------+--------+---------+---------+---------+------|
# | test1 |        |        |        |         |         |         |      |
# | test2 |        |        |        |         |         |         |      |
# | test3 |        |        |        |         |         |         |      |



import sys
import os


class Data:
    stage=0
    output_dict=dict()
    input_dict=dict()
    code=1
    def __init__(self):
        self.output_dict=dict()
        self.input_dict=dict()
    def Add(self, key, value):
        if self.stage == 1:
            # input
            self.input_dict[key] = value
        elif self.stage == 2:
            self.output_dict[key] = value
        else:
            pass
    def StartInput(self):
        self.stage = 1
    def StartOutput(self):
        self.stage = 2
    def ClearOutput(self):
        output_dict.clear()
    def SetCode(self, code):
        self.code = code
    def clear(self):
        self.input_dict.clear()
        self.output_dict.clear()

class Table:
    rows=[]
    def AddRow(self, row):
        self.rows.append(row)
    def gather_all_input(self):
        inputs=set()
        for row in self.rows:
            keys = row.input_dict.keys()
            for key in keys:
                inputs.add(key)
        return inputs
    def gather_all_output(self):
        outputs=set()
        for row in self.rows:
            keys = row.output_dict.keys()
            for key in keys:
                outputs.add(key)
        return outputs
    def get_header(inputs, outputs):
        lst=[]
        # the input_ and output_ prefix will be added
        for i in inputs:
            lst.append('input_' + i)
        for o in outputs:
            lst.append('output_' + o)
        lst.append("code")
        return ",".join(lst);
    def get_row(row, inputs, outputs):
        i_d=row.input_dict
        o_d=row.output_dict
        lst=[]
        for i in inputs:
            if i in i_d:
                lst.append(i_d[i])
            else:
                lst.append("NA")
        for o in outputs:
            if o in o_d:
                lst.append(o_d[o])
            else:
                lst.append("NA")
        lst.append(str(row.code))
        return ",".join(lst)
    def ToCSV(self):
        # gather all the input and output of rows
        # if a field is not there, put NA
        # output CSV File
        inputs = self.gather_all_input()
        outputs = self.gather_all_output()
        ret=""
        ret+=Table.get_header(inputs, outputs)
        for row in self.rows:
            ret += "\n"
            ret+=Table.get_row(row, inputs, outputs)
        return ret
    

def parse(filename):
    data = Data()
    table = Table()

    f = open(filename)
    for line in f:
        if line.find("HELIUM_INPUT_SPEC_END") is not -1:
            # end of input spec
            data.SetCode(3);
        elif line.find("HELIUM_INPUT_SPEC") is not -1:
            # start of input spec
            data.StartInput();
            data.SetCode(2);
        elif line.find("HELIUM_POI_INSTRUMENT_END") is not -1:
            # end of output instrument
            # start of POI
            data.SetCode(5);
        elif line.find("HELIUM_POI_INSTRUMENT") is not -1:
            # start of output instrument
            data.StartOutput()
            data.SetCode(4)
        elif line.find("HELIUM_AFTER_POI") is not -1:
            # POI has passed peacefully
            data.SetCode(6);
        elif line.find("HELIUM_TEST_SUCCESS") is not -1:
            # record the return code
            data.SetCode(7)
            table.AddRow(data)
            # start new row
            data = Data()
        elif line.find("HELIUM_TEST_FAILURE") is not -1:
            table.AddRow(data)
            data = Data()
        elif line.find("=") is not -1:
            key=line.split('=')[0].strip();
            value=line.split('=')[1].strip();
            data.Add(key, value);
    return table

if (len(sys.argv) == 1):
    print("Provide a file to parse")
    sys.exit(0)
    
filename=sys.argv[1]
# if (os.path.exists(filename)):
#     print("file " + filename + " does not exist.")
#     sys.exit(0)
table=parse(filename)



csv=table.ToCSV()
print(csv)
        


