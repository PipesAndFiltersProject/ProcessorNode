//
//  DataHandler.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 14.10.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

#include <string>

namespace OHARBase {
    class Package;
    
    /**
     DataHandler is an abstract class for handling data arriving to a ProcessorNode.
     Datahandler consumes data packages the ProcessorNode offers to it.
     Create new data handlers by inheriting DataHandler and implementing the consume method.
     ProcessorNode has a collection of data handlers to process all incoming data. To create a node
     means then to design and implement a set of handlers and put them in a suitable order in the collection
     and let the handlers do the processing of data.
     @todo Analyze if could use Visitor pattern to handle data in handlers. Handler accepts
     packages, both are abstract, double dispatch is used to match data/hander. ProcessorNode
     then would take the incoming data and pass it to data handlers. No need to dynamic_casts after that?
     @author Antti Juustila
     */
    class DataHandler {
    public:
        DataHandler(const DataHandler &) = delete;
        const DataHandler & operator =(const DataHandler &) = delete;
        
        virtual ~DataHandler();
        virtual bool consume(Package & data) = 0;
        
    protected:
        DataHandler();
        
    private:
        
    };
    
    
}
