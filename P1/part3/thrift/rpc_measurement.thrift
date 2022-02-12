struct ComplexDataStructure {
	1: i32 val1,
  2: double val2,
	3: string val3
}


service CustomSvc {

  string getHelloMessage(1: string message);

  string AcceptInt(1: i32 num);

  string AcceptDouble(1: double num);

  string AcceptString(1: string string_message);

  string AcceptComplexDataStructure(1: ComplexDataStructure complex_data);

  string AcceptClientSideStream(1: string streaming_string);

}
