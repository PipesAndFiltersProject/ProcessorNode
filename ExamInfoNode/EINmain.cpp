//
//  main.cpp
//  ExamInfoNode
//
//  Created by Antti Juustila on 27.1.2014.
//  Copyright (c) 2014 Antti Juustila. All rights reserved.
//

#include <iostream>

#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/Log.h>
#include <OHARStudentLayer/StudentDataItem.h>
#include <OHARStudentLayer/StudentNetInputHandler.h>
#include <OHARStudentLayer/StudentNetOutputHandler.h>
#include <OHARStudentLayer/StudentHandler.h>

int main(int argc, const char * argv[])
{
	using namespace OHARBase;
	
   Log::get().entry("main", "Launching %s", argv[0]);
   Log::get().entry("main", "Arguments: %d", argc);
	
	std::string configFile;
	if (argc > 1) {
		ProcessorNode * processor = new ProcessorNode(argv[0]);
		Log::get().entry("main", "arg1: %s", argv[1]);
		configFile = argv[1];
		processor->configure(configFile);
		
		using namespace OHARStudent;
		processor->addHandler(new StudentNetInputHandler());
		processor->addHandler(new StudentHandler(*processor));
		processor->addHandler(new StudentNetOutputHandler(*processor));
		
		processor->start();
		delete processor;
	} else {
		std::cout << "No config file specified! Give config file name as startup parameter." << std::endl;
	}

	return 0;
}
