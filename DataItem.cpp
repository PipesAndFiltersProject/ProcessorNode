//
//  DataItem.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 5.12.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include "DataItem.h"

namespace OHARBase {
	
	/** Default constructor, does nothing. */
	DataItem::DataItem() {
	}
	
	/** Copy constructor, copies the id of the object. */
	DataItem::DataItem(const DataItem &d)
	: id(d.id)
	{
	}
	
	/** The destructor, does nothing. */
	DataItem::~DataItem() {
	}
	
	/** Gets the id of the object.
	 @return The data item's id. */
	const std::string & DataItem::getId() const {
		return id;
	}
	
	/** Sets the id of the data item.
	 @param theId The new id for the data item. */
	void DataItem::setId(const std::string & theId) {
		id = theId;
	}
	
	/** Compares two data items and if they have the same id, returns true.
	 @return Returns true if the items are identical (id's match). */
	bool DataItem::operator == (const DataItem & item) const {
		return id == item.id;
	}
	
	/** Compares two data items and if they have the same id, returns false.
	 @return Returns false if the items are identical (id's match). */
	bool DataItem::operator != (const DataItem & item) const {
		return id != item.id;
	}
	
	
} //namespace