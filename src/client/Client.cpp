#include "Client.hpp"

// constructor && destructor
Client::Client(unsigned int fd, sockaddr_in clientAddrInfo) \
	: mFd(fd), \
	mIpAddress(clientAddrInfo.sin_addr.s_addr), \
	mPasswordConfirmation(false) ,
	mRegisterStatus(0)
{
	struct hostent	*host;

	mIpAddressString = inet_ntoa(clientAddrInfo.sin_addr);
	host = gethostbyaddr(&clientAddrInfo.sin_addr, sizeof(clientAddrInfo.sin_addr), AF_INET);
	if (host)
	{
		mHostName = host->h_name;
	}
	else
	{
		mHostName = "Unknownhost";
	}
}

Client::~Client() {}

// public member function(getter)

const std::string	&Client::GetNickName() const
{
	return (mNickName);
}

const std::string	&Client::GetUserName() const
{
	return (mUserName);
}

const std::string	&Client::GetHostName() const
{
	return (mHostName);
}

const std::string	&Client::GetServerName() const
{
	return (mServerName);
}

const std::string	&Client::GetRealName() const
{
	return (mRealName);
}

std::string	Client::GetClientPrefix() const
{
	return mNickName + "!" + mUserName + "@" + mHostName + " ";
}

unsigned int		Client::GetFd() const
{
	return (mFd);
}

in_addr_t			Client::GetIpAddress() const
{
	return (mIpAddress);
}

const std::string&	Client::GetIpAddressString() const
{
	return (mIpAddressString);
}

bool				Client::GetPasswordConfirmation() const
{
	return (mPasswordConfirmation);
}

int					Client::GetRegisterStatus() const
{
	return (mRegisterStatus);
}

// public member function(setter)

void				Client::SetNickName(const std::string &nickName)
{
	mNickName = nickName;
}

void				Client::SetUserName(const std::string &userName)
{
	mUserName = userName;
}

void				Client::SetHostName(const std::string &hostName)
{
	mHostName = hostName;
}

void				Client::SetServerName(const std::string &serverName)
{
	mServerName = serverName;
}

void				Client::SetRealName(const std::string &realName)
{
	mRealName = realName;
}

void				Client::SetPasswordConfirmation(const bool passwordConfirmation)
{
	mPasswordConfirmation = passwordConfirmation;
}

void	Client::AddBuffer(const std::string& buff)
{
	mbuffer = mbuffer + buff;

	AMessage*	message;

	while (checkCommand())
	{
		message = makeCommand();
		if (message != NULL)
		{
			message->ExecuteCommand();
			delete message;
		}
	}
}

bool	Client::checkCommand() const
{
	size_t	length;

	length = mbuffer.length();
	if (length >= 512)
		return 1;
	else if (mbuffer.find("\r\n") != std::string::npos)
		return 1;
	return 0;
}

AMessage*	Client::makeCommand()
{
	size_t		length;
	char		buff[513];
	AMessage*	message;

	length = mbuffer.length();
	if (length >= 512)
	{
		mbuffer.copy(buff, 512, 0);
		mbuffer.erase(0, 512);
		buff[510] = '\r';
		buff[511] = '\n';
		buff[512] = '\0';
	}
	else
	{
		std::string::size_type	pos = mbuffer.find("\r\n");
		mbuffer.copy(buff, pos + 2, 0);
		mbuffer.erase(0, pos + 2);
		buff[pos + 2] = '\0';
	}
	message = AMessage::GetMessageObject(this, buff);
	return message;
}

void		Client::TurnOnRegisterStatus(int mode)
{
	if (mode < 0 || mode > 3 || GetRegisterStatus() & (1 << mode))
	{
		return ;
	}
	mRegisterStatus |= (1 << mode);
	if (GetRegisterStatus() == REGISTERD)
	{
		std::string	result;

		result = Server::GetServer()->GetPrefix() + RPL_WELCOME(GetNickName(), GetUserName(), GetHostName());
		Server::GetServer()->SendMessage(*this, result);
	}
}
