#include "Join.hpp"

/* exception class */
Join::NoSuchChannelException::~NoSuchChannelException() throw() {}
Join::NoSuchChannelException::NoSuchChannelException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::NoSuchChannelException::what() const throw()
{
	return (ERR_NOSUCHCHANNEL(mChannelName));
}

Join::TooManyChannelsException::~TooManyChannelsException() throw() {}
Join::TooManyChannelsException::TooManyChannelsException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::TooManyChannelsException::what() const throw()
{
	return (ERR_TOOMANYCHANNELS(mChannelName));
}

Join::NeedMoreParamException::~NeedMoreParamException() throw() {}
std::string	Join::NeedMoreParamException::what() const throw()
{
	return (ERR_NEEDMOREPARAMS("JOIN"));
}

Join::ChannelIsFullException::~ChannelIsFullException() throw() {}
Join::ChannelIsFullException::ChannelIsFullException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::ChannelIsFullException::what() const throw()
{
	return (ERR_CHANNELISFULL(mChannelName));
}

Join::InviteOnlyChanException::~InviteOnlyChanException() throw() {}
Join::InviteOnlyChanException::InviteOnlyChanException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::InviteOnlyChanException::what() const throw()
{
	return (ERR_INVITEONLYCHAN(mChannelName));
}

Join::BannedFromChanException::~BannedFromChanException() throw() {}
Join::BannedFromChanException::BannedFromChanException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::BannedFromChanException::what() const throw()
{
	return (ERR_BANNEDFROMCHAN(mChannelName));
}

Join::BadChannelKeyException::~BadChannelKeyException() throw() {}
Join::BadChannelKeyException::BadChannelKeyException(const std::string &channelName) \
	: mChannelName(channelName) {}
std::string	Join::BadChannelKeyException::what() const throw()
{
	return (ERR_BADCHANNELKEY(mChannelName));
}

/* constructor && destructor */
Join::Join(Client* origin, const std::string &msg) : \
	AMessage(origin, "JOIN", msg) \
{}

Join::~Join()
{
}

void	Join::ExecuteCommand()
{
	try
	{
		parseParameter(getParameter());
	}
	catch (const NewException &e)
	{
		std::cerr << e.what() << std::endl;
		ReplyToOrigin(e.what());
	}

	std::map<const std::string, Channel>	&channelsInServer = \
		(Server::GetServer())->GetChannelList();

	while (!mChannelList.empty())
	{
		std::string	targetName = mChannelList.front();
		mChannelList.pop_front();

		std::map<const std::string, Channel>::iterator	target = \
			channelsInServer.find(targetName);

		// 채널이 없으면 새로 만든 후 최초로 join을 호출한 사람을 관리자로 삼는다.
		if (target == channelsInServer.end())
			channelsInServer[targetName] = Channel(targetName, *mOrigin);
		// 채널이 이미 존재할 경우
		else
		{
			try
			{
				// key 목록에 남은 키가 없다면 키 없이 채널에 가입을 시도한다.
				if (mChannelKeyList.empty())
					channelsInServer[targetName].AddUser(*mOrigin);
				// key 목록에 남은 키가 있다면 해당 키로 채널에 가입을 시도한다.
				else
				{
					channelsInServer[targetName].AddUserWithKey(*mOrigin, mChannelKeyList.front());
					mChannelKeyList.pop_front();
				}
				// 채널 가입에 실패하였다면 위에서 throw 된다.
			}
			// 채널 가입에 실패하였다면 에러 메세지를 서버에서 출력하고 클라이언트에게 보낸다.
			catch(const NewException &e)
			{
				std::cerr << e.what() << std::endl;
				ReplyToOrigin(e.what());
				continue ;
			}
		}
		// 채널 가입에 성공했으므로 채널의 모두에게 브로드캐스트한다.
		channelsInServer[targetName].Broadcast(*mOrigin, "Join Success!!");
	}
}

/* Join must not be called with default constructor(instance needs param) */
Join::Join() : \
	AMessage(NULL, "", std::string())
{}

/* private member function */
// 문자열에 제어문자가 포함이 되어 있는가?
bool	Join::isControlCharInString(const std::string &s)
{
	for (char controlChar = 1; controlChar < 32; controlChar++)
	{
		if (s.find(controlChar) != std::string::npos)
			return (true);
	}

	if (s.find(127) != std::string::npos)
		return (true);

	return (false);
}

// channel의 이름이 문법을 준수하였는가?
bool	Join::isChannelNameValid(const std::string &channelName)
{
	// channel 이름은 비어있거나 50자를 초과할 수 없다.
	if (channelName.empty() || channelName.size() > 50)
		return (false);

	// channel 이름은 #, &, + 중 하나의 문자로 시작해야 한다.
	if (channelName[0] != '#' && channelName[0] != '&' \
		&& channelName[0] != '+')
		return (false);

	// channel 이름엔 제어 문자가 포함될 수 없다.
	if (isControlCharInString(channelName) == true)
		return (false);

	return (true);
}

bool	Join::isChannelKeyValid(const std::string &channelKey)
{
	// key의 길이는 1 ~ 23
	if (channelKey.size() < 1 || channelKey.size() > 23)
	{
		std::cout << channelKey.size() << std::endl;
		return (false);
	}

	// key에는 아래의 아스키 문자가 포함될 수 없음
	char	bannedCharList[] = {6, 9, 10, 11, 13, 32};
	for (int i = 0; i < 6; i++)
	{
		if (channelKey.find(bannedCharList[i]) != std::string::npos)
			return (false);
	}

	return (true);
}

// parse parameter from msg
// prefix, command가 보장된 상태로 들어오므로 parameter만 파싱
std::string	Join::getParameter()
{
	// 캐리지 리턴을 지운다.
	mBuff.erase(mBuff.length() - 3);
	// prefix가 있으면 공백을 두 번 건너뛴다.
	if (mBuff[0] == ':')
		return (mBuff.substr(mBuff.find(" ", mBuff.find(" ") + 1) + 1));
	// 그렇지 않다면 한 번 건너뛴다.
	else
		return (mBuff.substr(mBuff.find(" ") + 1));
}

// ' '로 파라미터를 채널 목록과 키 목록으로 구분
// 파라미터는 # 또는 &로 시작해야 함
// 그렇지 않은 parameter 이후로는 모두 key 목록(mChannelKeyList)에 저장
void				Join::parseParameter(const std::string &parameter)
{
	std::istringstream	lineParser(parameter);
	std::string			channelParam;
	std::string			keyParam;

	// 파라미터가 비어 있다 -> ERR_NEEDMOREPARAM
	if (parameter.empty())
		throw (Join::NeedMoreParamException());

	// ' ' 구분자를 기준으로 파라미터의 채널 부분 분리
	std::getline(lineParser, channelParam, ' ');

	// ' ' 구분자를 기준으로 파라미터의 키 부분 분리(이 부분은 비어있을 수 있음)
	if (!lineParser.eof())
		std::getline(lineParser, keyParam, ' ');

	// 파라미터가 3개 이상이다 -> 3번째 파라미터부터 버리고 다음 로직으로 넘어간다.
	// 파라미터가 넘칠 경우의 err reply가 정의되어 있지 않으며
	// 해당 케이스에서 상용 프로그램도 정상적으로 작동함

	std::istringstream	channelListParser(channelParam);
	std::istringstream	keyListParser(keyParam);
	std::string			temp;

	while (!channelListParser.eof())
	{
		std::getline(channelListParser, temp, ',');
		if (isChannelNameValid(temp) == true)
			mChannelList.push_back(temp);
		else
			throw (Join::NoSuchChannelException(temp));
	}

	if (keyParam.empty())
		return ;

	while (!keyListParser.eof())
	{
		std::getline(keyListParser, temp, ',');
		if (isChannelKeyValid(temp) == true)
			mChannelKeyList.push_back(temp);
		else
			throw (Join::BadChannelKeyException(temp));
	}
}
