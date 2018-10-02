//
//  DataItem.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 5.12.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersFramework__DataItem__
#define __PipesAndFiltersFramework__DataItem__

#include <ostream>
#include <map>
#include <vector>

namespace OHARBase {


/**
 An abstract class, representing the data items the applications can use.
 Application developers should subclass the DataItem class and reimplement
 the abstract methods.
 This class implements the Prototype design pattern.
 @author Antti Juustila
 */
class DataItem {
public:
   DataItem();
   DataItem(const DataItem &d);
   virtual ~DataItem();
   
   virtual const std::string & getId() const;
   virtual void setId(const std::string & theId);

	/** Abstract method. Subclasses should implement how data is parsed from a string
	 to subclass specific data members. 
	 @param fromString The actual data to parse. 
	 @param contentType Application specific content type, used in parsing to determine what
	 kind of data should be expected to be in fromString. */
   virtual bool parse(const std::string & fromString, const std::string & contentType) = 0;
	/** Adds data from another data item to this data item. Instead of using assignment
	 operator, this method is offered since it does the necessary dynamic casts to determine
	 if the data types are compatible or not and assigns only from the same data type. This should
	 be of course implemented by app developers subclassing this class.
	 @param another An object from where data is copied to this object.
	 @return Returns true if the data items were copied, otherwise should return false. */
   virtual bool addFrom(const DataItem & another) = 0;
	
	/** The copy method implements the Prototype design pattern. It is useful to be able
	 to copy e.g. permanent data objects from temporary data objects. 
	 @return Returns a new copy of an exiting object. */
   virtual DataItem * copy() const = 0;
   
   bool operator == (const DataItem & item) const;
   bool operator != (const DataItem & item) const;

protected:
	/** The id for the data item. This is application specific information.*/
   std::string id;
private:
};
	
} //namespace

#endif /* defined(__PipesAndFiltersFramework__DataItem__) */
