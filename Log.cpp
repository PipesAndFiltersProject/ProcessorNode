//
//  Log.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 21.10.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <OHARBaseLayer/Log.h>

namespace OHARBase {

   
	/** Destructor deallocates the char buffer used in logging. */
	Log::~Log() {
		delete [] buffer;
	}
	
	/** The class method for retrieving the single instance of the logger.
	 @return The singleton logger object. */
	Log & Log::get() {
		static Log logInstance;
		return logInstance;
      
	}
	
	/** Use this method to set the output stream of the logger.
	 @param os The output stream to use in logging. */
	void Log::setStream(std::ostream & os) {
		stream = &os;
	}
	

	/** The hidden default constructor prevents instantiating from elsewhere
	 than the getInstance() method. */
	Log::Log()
	: stream(&std::cout), bufSize(1000)
	{
		buffer = new char[bufSize];
		started = std::chrono::system_clock::now();
	}
	
   std::string Log::getTimestamp() {
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      
      std::stringstream s;
      s << std::setw(10) << std::chrono::duration_cast<std::chrono::milliseconds>(now - started).count();
      return s.str();
   }
   
   
   Log & Log::info(const std::string & TAG) {
      Log::get().lock();
      Log::get() << Log::get().getTimestamp() << " " << TAG;
      Log::get().unlock();
      return Log::get();
   }
   
   void Log::lock() {
      
   }
   
   void Log::unlock() {
      
   }
   	
}	// namespace
