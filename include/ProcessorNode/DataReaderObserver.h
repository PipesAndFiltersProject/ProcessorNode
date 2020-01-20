//
//  DataReaderObserver.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2014.
//  Copyright (c) 2014 Antti Juustila. All rights reserved.
//

#ifndef PipesAndFiltersFramework_DataReaderObserver_h
#define PipesAndFiltersFramework_DataReaderObserver_h

namespace OHARBase {
   
   // Forward declaration.
   class DataItem;
   
   /** Defines the interface for observing the reading of data items.
    The observer will get a notification when data items are read.
    */
   class DataReaderObserver {
   public:
      /** This method is called in the observer when a new data item was
       successfully created. The ownership of the data item object is transferred
       to the observer.
       @param item The new data item object created, owned now by the observer. */
      virtual void handleNewItem(std::unique_ptr<DataItem> item) = 0;
      virtual ~DataReaderObserver() { };
   };
   
   
} //namespace
#endif
