#include "ReplyMacros.hpp"
#include "Privmsg.hpp"
#include "Server.hpp"

Privmsg::Privmsg(Client* origin, const std::string msg) :
AMessage(origin, "Privmsg", msg)
{
}

Privmsg::~Privmsg()
{
}

void Privmsg::ExecuteCommand()
{
	ParseBuffer();
	if (mTarget.front() == ':')
	{	
		ReplyToOrigin(ERR_NORECIPIENT(mOrigin->GetNickName()));
		return ;
	}
	else if (mMessage.empty() == true)
	{
		ReplyToOrigin(ERR_NOTEXTTOSEN);
		return ;
	}
	if (mTarget.front() == '#')
	{
		SendMessageChannel();
	}
	else
	{
		SendMessageClient();
	}
}

void Privmsg::ParseBuffer()
{
	size_t		delim;
	std::string	msg_only_param;

	delim = mBuff.find(" ");
	msg_only_param = mBuff.substr(delim, mBuff.length());
	delim = msg_only_param.find(" :");
	if (delim == std::string::npos)
	{
		mTarget = msg_only_param;
		return ;
	}
	mTarget = msg_only_param.substr(0, delim);
	mMessage = msg_only_param.substr(delim + 1, msg_only_param.length());
}

void Privmsg::SendMessageChannel()
{	
	// if (there is no channel)
	// {
	// 	ERR_NOSUCHNICK
	// }
	// else if (channel is not invited)
	// {
	// 	ERR_CANNOTSENDTOCHAN
	// }
}

void Privmsg::SendMessageClient()
{
	Client* target = Server::GetServer()->ReturnClientOrNull(mTarget);

	if (target == NULL)
	{
		ReplyToOrigin(ERR_NOSUCHNICK(mOrigin->GetNickName()));
	}
	SendPrivMessage(target);
}

void Privmsg::SendPrivMessage(Client* target)
{
	std::string	result;

	result = GetPrefix() + mBuff;
	Server::GetServer()->SendMessage(*target, result);
}
