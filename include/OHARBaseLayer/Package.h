//
//  Package.h
//  PipesAndFiltersStudentNode
//
//  Created by Antti Juustila on 28.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersStudentNode__Package__
#define __PipesAndFiltersStudentNode__Package__

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <nlohmann/json.hpp>


namespace OHARBase {
    
    // Forward declaration.
    class DataItem;
    
    /**
     Package encapsulates the data send between the ProcessorNode objects as well
     as passing the data around within a ProcessorNode and the Handlers and readers/writers
     in the node.<p>
     Each package has a unique identifier which can be used to identify data packages.
     Package also has a type, indicating is the package containing actual data to be
     handled, or is the package containing a control message. Control messages influence
     on the way the Node behaves. These include e.g. shutting down the node.
     @todo Add a "corrupt" flag for packages that have bad content. These packages
     are just passed through the Filters, logging their presence, and they are not
     handled by any Handler. Would aid in debugging/testing since corrupt packages would
     not just disappear.
     @author Antti Juustila
     */
    class Package {
    public:
        /*! The type of the package. */
        enum Type {
            NoType,   /*!< The package has no type (yet); it is uninitialized. */
            Control,  /*!< The package contains a control message. */
            Data      /*!< The package contains actual application specific data. */
        };
        
        Package();
        Package(const Package & p);
        Package(Package && p);
        Package(const boost::uuids::uuid & id);
        Package(Type ptype, const std::string & data);
        Package(const boost::uuids::uuid & id, Type ptype, const std::string & data);
        virtual ~Package();
        
        const boost::uuids::uuid & getUuid() const;
        void setUuid(const boost::uuids::uuid & id);
        Type getType() const;
        void setType(Type ptype);
        const std::string & getData() const;
        void setData(const std::string & d);
        const DataItem * getDataItem() const;
        DataItem * getDataItem();
        void setDataItem(const DataItem * item);
        
        bool isEmpty() const;
        const Package & operator = (const Package & p);
        const Package & operator = (Package && p);
        bool operator == (const Package & pkg) const;
        bool operator == (const std::string & str) const;
        
        static const std::string & separator();
        //		bool parse(const std::string & buffer);
        
        const std::string & getTypeAsString() const;
        void setTypeFromString(const std::string & typeStr);
        
    private:
        /** The unique identifier for a package. Generated using the boost library support. */
        boost::uuids::uuid uid;
        /** Type of the data package. It is either Control or Data. Control packages are
         commands to both ProcessorNode and DataHandler objects on how they are controlled.
         Commands are:<br />
         <ul><li>ping, which causes the Node to print out a ping command,</li>
         <li>readfile, which causes the Node to read it's data file if it has been configured to do so,</li>
         <li>shutdown, which causes the node to shut down itself in a controlled way.</li></ul>
         All the commands are also forwarded to the next Node in the Pipes & Filters architecture.
         Thus all Nodes can be shut down by writing the command "shutdown" in the console of the first Node.<p>
         Data packages are application specific data items. */
        Type type;
        /** Data as received from the network/sent to the network.*/
        std::string data;
        /** Parsed data, structure of which is application specific. Application developers
         subclass their data objects from DataItem and implement application specific data
         structures in their subclasses. Parsing of data from Package::data to Package::dataItem
         happens in other applicatin specific classes.<br />
         This member is null if nothing has been parsed, and not null if the pointer points to
         a data item parsed from input (either from network or from file, for example).*/
        DataItem *dataItem;
        
        /** The character separator used to separate the data items in the string
         representation of the data. Used when data is received as string and when
         data is sent to the next Node. */
        static const std::string separatorStr;
        /** Textual representation of the package type Package::Control. */
        static const std::string controlStr;
        /** Textual representation of the package type Package::Data. */
        static const std::string dataStr;
        /** Textual representation of the package type Package:NoType. */
        static const std::string noTypeStr;
    };
    
    void to_json(nlohmann::json & j, const Package & package);
    void from_json(const nlohmann::json & j, Package & package);
    
    
} //namespace
#endif /* defined(__PipesAndFiltersStudentNode__Package__) */
