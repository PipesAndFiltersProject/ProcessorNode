//
//  DataFileReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 5.12.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <fstream>

#include <g3log/g3log.hpp>

#include <ProcessorNode/DataFileReader.h>
#include <ProcessorNode/DataReaderObserver.h>




namespace OHARBase {
    
    const std::string DataFileReader::TAG = "FileReader ";
    
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
        LOG(INFO) << TAG << "Starting to handle the file " << fileName;
        std::ifstream file(fileName, std::ifstream::in);
        if (!file.is_open()) {
            LOG(WARNING) << TAG << "Could not open the file!!";
            return false;
        }
        std::string str;
        std::string contentType;
        std::getline(file, contentType);
        do {
            std::getline(file, str);
            if (file.good() && str.length() > 0) {
               std::unique_ptr<DataItem> item = parse(str, contentType);
                if (item) {
                   observer.handleNewItem(std::move(item));
                }
            }
        } while (file.good());
        file.close();
        LOG(INFO) << TAG << "File read finished.";
        return true;
    }
    
    
} //namespace
