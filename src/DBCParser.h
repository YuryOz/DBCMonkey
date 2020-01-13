/*
 * DBCParser.h
 *
 *  Created on: Jan 11, 2020
 *      Author: YuryOz
 */

#ifndef SRC_DBCPARSER_H_
#define SRC_DBCPARSER_H_
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <regex>

typedef std::unordered_map<uint32_t, std::string> IntStrMap;
typedef std::unordered_map<std::string, uint32_t> StrIntMap;


typedef struct {
	uint32_t msgID;
	uint32_t ID;
	std::string Name;
	uint32_t BitPos;
	uint32_t BitSize;
	float Scale;
	float Offset;
	float Min;
	float Max;
	std::string Units;
	std::string Receivers;
	bool isLE; // little endian
	bool isMP; // multiplex
	bool isSigned;
	int32_t MPvalue;
	std::string MPSigName;
	IntStrMap EnumValues;
} DBCSignal;

typedef std::unordered_map<std::string, DBCSignal> SignalMap;  // signal name : DBCSignal

typedef struct {
	uint32_t ID;
	std::string Name;
	uint32_t SizeBytes;
	std::string Senders;
	SignalMap Signals;
} DBCMessage;

typedef std::unordered_map<uint32_t, DBCMessage> MessageMap; // msg ID : DBCMessage

typedef std::unordered_map<uint32_t, MessageMap> ChannelMap;

typedef std::unordered_map<uint32_t, StrIntMap> MessageNameMap;


class DBCParser {
	ChannelMap mData;
	MessageNameMap mNamesData;

public:
	DBCParser();
	virtual ~DBCParser();
	bool ReadDBC(const std::string &filename, uint32_t channel = 0);
	void Print();

	std::string &getMessageName(const uint32_t message_id, const uint32_t channel = 0);
	uint32_t getMessageId(const std::string &message_name, const uint32_t channel = 0);

	std::vector<uint32_t> getChannels();
	std::vector<uint32_t> getMessages(const uint32_t channel = 0);
	std::vector<std::string> getSignals(const uint32_t message_id, const uint32_t channel = 0);
	std::vector<std::string> getSignals(const std::string & message_name, const uint32_t channel = 0);

	DBCSignal &getSignal(const std::string &signal_name, const uint32_t message_id, const uint32_t channel = 0);
	DBCSignal &getSignal(const std::string &signal_name, const std::string &message_name, const uint32_t channel = 0);

};

#endif /* SRC_DBCPARSER_H_ */
