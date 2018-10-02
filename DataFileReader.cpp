//
//  DataFileReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 5.12.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <fstream>

#include "DataFileReader.h"
#include "DataReaderObserver.h"
#include "Log.h"


namespace OHARBase {
	
	const std::string DataFileReader::TAG = "FileReader";
	
	/** Constructor for the reader, providing the compulsory observer who
	 gets notified of new DataItem objects created when reading and parsing the file.
	 @param obs The observer
	 */
	DataFileReader::DataFileReader(DataReaderObserver & obs)
	: observer(obs)
	{
	}
	
	/** Destructor, does nothing. */
	DataFileReader::~DataFileReader() {
	}
	
	/** Reads lines from the file, parses the lines one by one to create DataItem objects from
	 the lines. Notifies the observer whenever a DataItem object was successfully created
	 from the line, providing the data object to the observer.
	 @param fileName The file to open, read, parse and close.
	 @return Returns true if file was successfully handled, false if file couldn't be opened.
	 */
	bool DataFileReader::read(const std::string &fileName) {
		Log::getInstance().entry(TAG, "Starting to handle the file %s", fileName.c_str());
		std::ifstream file(fileName, std::ifstream::in);
		if (!file.is_open()) {
			Log::getInstance().entry(TAG, "Could not open the file!!");
			return false;
		}
		std::string str;
		std::string contentType;
		std::getline(file, contentType);
		do {
			std::getline(file, str);
			Log::getInstance().entry(TAG, "Read line: %s", str.c_str());
			if (file.good() && str.length() > 0) {
				DataItem * item = parse(str, contentType);
				if (item) {
					observer.handleNewItem(item);
				}
			}
		} while (file.good());
		file.close();
		return true;
	}
	
	
} //namespace