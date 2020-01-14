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

bool DBCParser::ReadDBC(const std::string &filename, const uint32_t channel) {

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
				mNamesData[channel][msg.Name] = msg.ID;

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

DBCSignal &DBCParser::getSignal(const std::string &signal_name, const std::string &message_name, const uint32_t channel){
	return getSignal(signal_name, getMessageId(message_name, channel), channel);
}

std::string &DBCParser::getMessageName(const uint32_t message_id, const uint32_t channel){
	return mData[channel][message_id].Name;
}

uint32_t DBCParser::getMessageId(const std::string &message_name, const uint32_t channel){
	return mNamesData[channel][message_name];
}

std::vector<std::string> DBCParser::getSignals(const uint32_t message_id, const uint32_t channel){
	std::vector<std::string> signals;
	signals.reserve(mData[channel][message_id].Signals.size());

	for (auto& sig_it: mData[channel][message_id].Signals) {
		signals.push_back(sig_it.first);
	}

	return signals;
}

std::vector<std::string> DBCParser::getSignals(const std::string &message_name, const uint32_t channel){
	return getSignals(getMessageId(message_name, channel), channel);
}

std::vector<uint32_t> DBCParser::getChannels() {
	std::vector<uint32_t> channels;
	channels.reserve(mData.size());
	for (auto &ch_it : mData) {
		channels.push_back(ch_it.first);
	}
	return channels;
}

std::vector<uint32_t> DBCParser::getMessages(const uint32_t channel){
	std::vector<uint32_t> messages;
	messages.reserve(mData[channel].size());
	for (auto &msg_it : mData[channel]) {
		messages.push_back(msg_it.first);
	}
	return messages;
}

double DBCParser::getDoubleValue(const std::string &signal_name, const uint32_t message_id, const uint32_t channel, const unsigned char* data, uint32_t size){
	double ret_value;
	DBCSignal &signal = getSignal(signal_name, message_id, channel);
	if((signal.BitPos + signal.BitSize) <=  (size * 8)) {
		uint32_t bytePos = signal.BitPos / 8;
		uint32_t startBitInByte = signal.BitPos % 8;

		uint32_t byteSize = (signal.BitSize + 7)/ 8;

		uint32_t mask = (1 << signal.BitSize) - 1;

		std::cout << message_id <<" Signal " << signal_name
				<< " bit pos " << signal.BitPos <<  " bit len " << signal.BitSize
				<< " Byte pos " << bytePos <<  " Byte len " << byteSize
				<< " StartInByte " << startBitInByte <<  std::endl;


		uint8_t out_data[4];
		uint32_t *out_data_ptr = (uint32_t *) &out_data[0];

		*out_data_ptr = 0;

		for(int i = 0; i < byteSize; i++) {
			out_data[i] = data[bytePos + i];
			printf("%d DATA 0x%02X\n", i, out_data[i]);
		}

		printf("MASK 0x%08X\n", mask);
		printf("DATA 0x%08X\n", *out_data_ptr);
		//TODO shift and calc value

	}

	return ret_value;
}


DBCSignal &DBCParser::getSignal(const std::string &signal_name, const uint32_t message_id, const uint32_t channel) {
	return mData[channel][message_id].Signals[signal_name];
}

void DBCParser::Print() {

	std::vector<uint32_t> channels = getChannels();

	std::cout <<  "Channels #" << channels.size() << std::endl;
	for(auto channel : channels) {
		std::cout <<  "Channel " << channel << std::endl;
		std::vector<uint32_t> messages = getMessages(channel);
		std::cout <<  "   Messages #" << messages.size() << std::endl;
		for(auto message:  messages) {
			std::cout <<  "   Message " << message << std::endl;
			std::vector<std::string> signals = getSignals(message, channel);
			std::cout <<  "      Signals# " << signals.size() << std::endl;
			for(auto sig: signals) {
				std::cout <<  "      Signal# " << sig << std::endl;
				DBCSignal &signal = getSignal(sig, message, channel);


				std::cout <<  "       Signal " << signal.Name << " " << signal.msgID << std::endl;
				std::cout <<  "       bits: " << signal.BitPos << " " <<  signal.BitSize << std::endl;
				std::cout <<  "       min: " << signal.Min << " max:" <<  signal.Max << std::endl;

				unsigned char data[8];

				data[0] = 0x12;
				data[1] = 0x34;
				data[2] = 0x56;
				data[3] = 0x78;
				data[4] = 0x9A;
				data[5] = 0xBC;
				data[6] = 0xDE;
				data[7] = 0xF0;

				double val = getDoubleValue(signal.Name, message, channel, &data[0], 8);

				std::cout <<  "       Value : " << val << std::endl;

			}
		}
	}
}


