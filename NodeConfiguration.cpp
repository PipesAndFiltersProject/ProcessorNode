//
//  NodeConfiguration.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 25.2.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include "NodeConfiguration.h"

namespace OHARBase {
	
	NodeConfiguration::~NodeConfiguration() {
		while (configItems.size() > 0) {
			delete configItems[0];
			configItems.erase(configItems.begin());
		}
	}
	
	void NodeConfiguration::handleNewItem(DataItem * item) {
		ConfigurationDataItem * d = dynamic_cast<ConfigurationDataItem*>(item);
		if (d) {
			configItems.push_back(d);
		}
	}
	
	std::string NodeConfiguration::getValue(const std::string &configName) const {
		for (ConfigurationDataItem * configItem : configItems) {
			if (configItem->getItemName() == configName) {
				return configItem->getItemValue();
			}
		}
		return "";
	}
	
}