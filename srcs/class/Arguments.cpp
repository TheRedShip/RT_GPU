/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Arguments.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 01:05:44 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 22:36:06 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

Arguments::Arguments(int argc, char **argv)
{
	initArguments();
	_headless = 0;
	_err = 0;

	if(argc <= 1)
	{
		_err = 1;
		printUsage();
		return ;
	}
	
	for(int i = 1; i < argc; i++)
	{
		if(!handleArg(argv, argc, &i))
		{
			_err = 1;
			return ;
		}
	}
	if(_values.find("sceneName") == _values.end())
	{
		std::cerr << "missing scene name" << std::endl;
		_err = 1;
	}
	if(getBoolean("server") && getBoolean("client"))
	{
		std::cerr << "RT can't be both a client and a server" << std::endl;
		_err = 1;
	}
}

void Arguments::initArguments()
{
	addArgument('r', "renderpath", 0);
	addArgument('h', "headless", 1);
	addArgument('c', "client", 0);
	addArgument('s', "server", 0);
}

void	Arguments::printUsage(void)
{ std::cerr << "usage : [options] <scene name> [options]" << std::endl;
	std::cerr << R""""(options :
	-r | --renderpath <file>: filename for the renderer path
	-h | --headless : does the program need to start rendering as soon as it starts(and close automatically)
	-c | --client <server ip:port> : start RT as a client
	-s | --server <port>: start a server
)"""";	
}

void	Arguments::show(void)
{
	for(std::map<std::string, std::string>::iterator it = _values.begin();it != _values.end(); it++)
		std::cout << (*it).first << ": " << (*it).second << std::endl;
}

bool	Arguments::error(void) const
{
	return(_err);
}


void	Arguments::addArgument(char shortName, std::string longName, int isFlag)
{
	t_arg arg;

	arg.shortName = shortName;
	arg.longName = longName;
	arg.isFlag = isFlag;
	_args.push_back(arg);
}

int	Arguments::setArg(t_arg arg, char **argv, int argc, int *i)
{

				if(arg.isFlag)
					_values[arg.longName] = "yes";
				else if(*i == argc - 1)
				{
					std::cerr << "missing option for " << arg.longName << std::endl;
					return(0);
				}
				else
					_values[arg.longName] = argv[++(*i)];
				return(1);
}

int Arguments::handleArg(char **argv, int argc, int *i)
{
	std::string arg(argv[*i]);
	std::vector<t_arg>::iterator it;

	(void)i;
	if(!arg.size())
		return(1);
	if(arg.size() >= 2 && arg[0] == '-' && arg[1] == '-')
	{
		for(std::vector<t_arg>::iterator it = _args.begin(); it != _args.end(); it++)
		{
			if((*it).longName == arg.substr(2))
				return(setArg((*it), argv, argc, i));
		}
		std::cerr<< "unrecognized option : " << arg << std::endl;
		return(0);
	}
	else if(arg[0] == '-')
	{
		for(size_t j = 1; j < arg.size(); j++)
		{
			for(it = _args.begin(); it != _args.end(); it++)
			{
				if((*it).shortName == arg[j])
				{
					if(!setArg((*it), argv, argc, i))
						return(0);
					break;
				}
			}
			if(it == _args.end())
			{
				std::cerr << "unrecognized option : -" << arg[j] << std::endl;
				return(0);
			}
		}
	}
	else
	{
		if(_values.find("sceneName") == _values.end())
			_values["sceneName"] = arg;
		else
		{
			std::cerr << "unrecognized option : " << arg << std::endl;
			return(0);
		}
	}
	return(1);
}

bool Arguments::getBoolean(std::string name)
{
	return(_values.find(name) != _values.end());	
}

std::string *Arguments::getString(std::string name)
{
	if(_values.find(name) != _values.end())
		return(&_values[name]);
	else
		return(0);
}

std::string &Arguments::getSceneName(void) 
{
	return(_values["sceneName"]);
}
