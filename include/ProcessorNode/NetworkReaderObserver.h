//
//  NetworkObserver.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 20.9.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

namespace OHARBase {
	
	
	/** Interface for observing the NetworkReader. Network reader notifies
	 the observer using this interface when data has arrived and is ready
	 for handling. 
	 @author Antti Juustila
	 */
	class NetworkReaderObserver {
	public:
		/** NetworkReader calls this interface method when data has been received.
		 The observer then reads the data from the NetworkReader. */
		virtual void receivedData() = 0;
        /** NetworkReader calls this method if it cannot parse/handle the data that was received.
         @param what What went wrong in data handling. */
      virtual void errorInData(const std::string & what) = 0;
	};
	
	
} // namespace

