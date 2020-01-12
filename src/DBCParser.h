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

typedef std::unordered_map<uint32_t, std::string> StrMap;



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
	StrMap EnumValues;
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




class DBCParser {
	ChannelMap mData;


public:
	DBCParser();
	virtual ~DBCParser();
	bool ReadDBC(const char *filename, uint32_t channel = 0);
	void Print();

	std::string &getMessageName(uint32_t message_id, uint32_t channel = 0);
	uint32_t getMessageId(const char* message_name, uint32_t channel = 0);

};

#endif /* SRC_DBCPARSER_H_ */
