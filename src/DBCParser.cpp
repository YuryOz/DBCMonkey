/*
 * DBCParser.cpp
 *
 *  Created on: Jan 11, 2020
 *      Author: YuryOz
 */

#include "DBCParser.h"

#define DBC_MSG_ID 2
#define DBC_MSG_NAME 3
#define DBC_MSG_SIZE 4
#define DBC_MSG_SENDERS 5

#define DBC_SIG_NAME 2
#define DBC_SIG_MULTIPLEX 3
#define DBC_SIG_BIT_POS 4
#define DBC_SIG_BIT_LEN 5
#define DBC_SIG_LE 6
#define DBC_SIG_SIGNED 7
#define DBC_SIG_SCALE 8
#define DBC_SIG_OFFSET 9
#define DBC_SIG_MIN 10
#define DBC_SIG_MAX 11
#define DBC_SIG_UNITS 12
#define DBC_SIG_RECEIVER 13

#define DBC_VAL_MSGID 2
#define DBC_VAL_SIG_NAME 3
#define DBC_VAL_DATA 4


DBCParser::DBCParser() {
	// TODO Auto-generated constructor stub

}

DBCParser::~DBCParser() {
	// TODO Auto-generated destructor stub
}

bool DBCParser::ReadDBC(const char *filename, uint32_t channel) {

	std::cout << "reading file: " << filename << " channel: " << channel << std::endl;
	std::ifstream dbcfile(filename);

	if (dbcfile.is_open()) {

		std::string line;
		uint32_t curMsgID = 0;
		std::string cur_mp_signal;

		while (std::getline(dbcfile, line)) {

			std::smatch m;
			std::regex msg_rgxp("^(BO_) (\\d+) ([^ ]+): (\\d+) (.*)$");
			std::regex sig_rgxp("^( SG_) ([^ ]+) (.*): (\\d+)\\|(\\d+)@(\\d)(\\+|\\-) \\((.*),(.*)\\) \\[(.*)\\|(.*)\\] \\\"(.*)\\\" (.*)$");
			std::regex sig_val_rgxp("^(VAL_) (\\d+) ([^ ]+) (.*);");
			std::regex sig_val_data_rgxp("(\\d+) \\\"([^\"]+)\\\" ");
			int n = 0;

			if (regex_search(line, m, msg_rgxp) && m.size() == 6){

				DBCMessage msg;
				msg.ID = stoul(m[DBC_MSG_ID].str());
				msg.Name = m[DBC_MSG_NAME].str();
				msg.SizeBytes = stoul(m[DBC_MSG_SIZE].str());
				msg.Senders = m[DBC_MSG_SENDERS].str();
				curMsgID = msg.ID;

				mData[channel][msg.ID] = msg;

//				for (auto x : m) {
//					std::cout << n++ << " MESSAGE [" << x << "] " << std::endl;
//				}

			} else if (regex_search(line, m, sig_rgxp) && m.size() == 14) {

				DBCSignal sig;
				sig.msgID = curMsgID;
				sig.Name = m[DBC_SIG_NAME].str();
				sig.BitPos =  stoul(m[DBC_SIG_BIT_POS].str());
				sig.BitSize =  stoul(m[DBC_SIG_BIT_LEN].str());
				sig.isLE = stoul(m[DBC_SIG_LE].str());
				sig.Scale = stof(m[DBC_SIG_SCALE].str());
				sig.Offset = stof(m[DBC_SIG_OFFSET].str());
				sig.Min = stof(m[DBC_SIG_MIN].str());
				sig.Max = stof(m[DBC_SIG_MAX].str());

				sig.Units = m[DBC_SIG_UNITS].str();
				sig.Receivers = m[DBC_SIG_RECEIVER].str();

				if(m[DBC_SIG_SIGNED].str() == "+"){
					sig.isSigned = true;
				} else { //"-"
					sig.isSigned = false;
				}

				sig.isMP = false;
				sig.MPvalue = -1;

				if( m[DBC_SIG_MULTIPLEX].str().size()) {

					if(m[DBC_SIG_MULTIPLEX].str() == "M ") {
						cur_mp_signal = m[DBC_SIG_NAME].str();
					} else if(m[DBC_SIG_MULTIPLEX].str()[0] == 'm') {
						sig.MPSigName = cur_mp_signal;
						sig.MPvalue = stoul(m[DBC_SIG_MULTIPLEX].str().substr(1));
						sig.isMP = true;
					}
				}
				mData[channel][curMsgID].Signals[sig.Name] = sig;

			} else if(regex_search(line, m, sig_val_rgxp)) {

				uint32_t msgID = stoul(m[DBC_VAL_MSGID].str());
				std::string sigName = m[DBC_VAL_SIG_NAME].str();
				std::string sigData = m[DBC_VAL_DATA].str();

				for(std::sregex_iterator i = std::sregex_iterator(sigData.begin(), sigData.end(), sig_val_data_rgxp); i != std::sregex_iterator(); ++i)
			    {
			        uint32_t numval = stoul((*i)[1].str());
			        std::string strval = (*i)[2].str();
			        mData[channel][msgID].Signals[sigName].EnumValues[numval] = strval;
			    }
			}
		}
	}
	return true;
}


std::string &DBCParser::getMessageName(uint32_t message_id, uint32_t channel){

	return mData[channel][message_id].Name;
}

uint32_t DBCParser::getMessageId(const char* message_name, uint32_t channel){

 return 0;
}


void DBCParser::Print() {

	for (auto& chan_it: mData) {
	    std::cout <<  "Channel " << chan_it.first  << " Msgs: " << chan_it.second.size() << std::endl;

	    for(auto & msg_it: chan_it.second) {
	    	std::cout <<  " + Message " << msg_it.first  << " " << msg_it.second.Name.c_str() << std::endl;

	    	for(auto & sig_it: msg_it.second.Signals) {
	    		std::cout <<  "     Signal " << sig_it.first.c_str() << " " << sig_it.second.msgID << std::endl;
	    		std::cout <<  "       bit pos " << sig_it.second.BitPos << " size " << sig_it.second.BitSize << std::endl;
	    		std::cout <<  "           min " << sig_it.second.Min << " max " << sig_it.second.Max << " scale " << sig_it.second.Scale << " off " << sig_it.second.Offset << std::endl;
	    		std::cout <<  "        signed " << sig_it.second.isSigned << " units " << sig_it.second.Units << std::endl;
	    		std::cout <<  "            LE " << sig_it.second.isLE << " mplex " << sig_it.second.isMP << " sig "  << sig_it.second.MPSigName << " val " << sig_it.second.MPvalue  << std::endl;

	    		for(auto & enval: sig_it.second.EnumValues) {
	    			std::cout <<  "             VAL: " << enval.first << " " << enval.second.c_str() << std::endl;
	    		}
	    	}
	    }
	}
}


