#pragma once

//Replys
#define RPL_WELCOME(NICK, USER, HOST) 		(std::string("001") + " Welcome to the Internet Relay Network " + NICK + "!" + USER + "@" + HOST + "\r\n")
#define RPL_TOPIC(CHANNEL, TOPIC) 			(std::string("332") + " " + CHANNEL + " :" + TOPIC + "\r\n")

//Errors

#define ERR_NOSUCHNICK(NICK)				(std::string("401 ") + NICK + " :No such nick/channel\r\n")
#define ERR_CANNOTSENDTOCHAN(CHAN)			(std::string("404 ") + CHAN + " :Cannot send to channel\r\n")
#define ERR_NORECIPIENT(COMMAND)			(std::string("411 ") + ":No recipient given " + COMMAND + "\r\n")
#define ERR_NOTEXTTOSEN						(std::string("412 :No text to send\r\n"))
#define ERR_NONICKNAMEGIVEN 				(std::string("431") + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(NICK)			(std::string("432") + " " + NICK + " :Erroneous nickname\r\n")
#define ERR_NICKNAMEINUSE(NICK)				(std::string("433") + " " + NICK + " :Nickname is already in use\r\n")
#define ERR_NICKCOLLISION(NICK, USER, HOST) (std::string("436") + " " + NICK + " :Nickname collision KILL from " + USER + "@" + HOST + "\r\n")
#define ERR_UNAVAILRESOURCE(NAME, TYPE) 	(std::string("437") + " " + NAME + " :" + TYPE + " is temporarily unavailable\r\n") //can be used for both nick and channel
#define ERR_NEEDMOREPARAMS(COMMAND) 		(std::string("461") + " " + COMMAND + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED				(std::string("462 :Unauthorized command (already registered)\r\n"))
#define ERR_BADCHANNELKEY(CHANNEL) 			(std::string("475") + " " + CHANNEL + " :Cannot join channel (+k)\r\n")
#define ERR_RESTRICTED						(std::string("484 :Your connection is restricted!\r\n"))
