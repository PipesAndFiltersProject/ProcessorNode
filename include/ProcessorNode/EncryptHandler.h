//
//  EncryptHandler.h
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 21.04.2020.
//  Copyright (c) 2013-2020 Antti Juustila. All rights reserved.
//

#pragma once


#include <ProcessorNode/DataHandler.h>

namespace OHARBase {

   class Package;

	/**
	 EncryptHandler encrypts data item's payload string data.
	 @author Antti Juustila
	 @see DataHandler
	 */
   class EncryptHandler : public OHARBase::DataHandler {
   public:
      /*! The mode of the encrypter */
      enum Mode {
         Encrypt, /*!< The mode is to encrypt clear text to encrypted text. */
         Decrypt /*!< The mode is to decrypt encrypted text to clear text. */
      };
		EncryptHandler(EncryptHandler::Mode mode);
		virtual ~EncryptHandler();
		
		bool consume(OHARBase::Package & data) override;
		
   private:
      void rot13(const std::string & source, std::string & destination);
      
	private:
		/** The tag used in the logging to indicate which object is logging now. */
		static const std::string TAG;
      
      /** The mode of the encrypter, see the Mode definition for values. */
      Mode encryptionMode;
	};
	
	
}
