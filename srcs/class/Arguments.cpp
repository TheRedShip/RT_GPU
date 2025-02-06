/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Arguments.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 01:05:44 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/04 17:10:24 by tomoron          ###   ########.fr       */
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
}

void Arguments::initArguments()
{
	addArgument('r', "renderpath", 0);
	addArgument('h', "headless", 1);
}

void	Arguments::printUsage(void)
{
	std::cerr << "usage : [options] <scene name> [options]" << std::endl;
	std::cerr << R""""(options :
	-r | --renderpath <file>: filename for the renderer path
	-h | --headless : does the program need to start rendering as soon as it starts(and close automatically)
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


std::string &Arguments::getSceneName(void) 
{
	return(_values["sceneName"]);
}

std::string *Arguments::getRenderPath(void)
{
	if(_values.find("renderpath") != _values.end())
		return(&_values["renderpath"]);
	else
		return(0);
}

bool		Arguments::getHeadless(void)
{
	return(_values.find("headless") != _values.end());	
}


void	Arguments::addArgument(char shortName, std::string longName, int isFlag)
{
	t_arg arg;

	arg.shortName = shortName;
	arg.longName = longName;
	arg.isFlag = isFlag;
	_args.push_back(arg);
}


int Arguments::handleArg(char **argv, int argc, int *i)
{
	std::string arg(argv[*i]);

	(void)i;
	if(!arg.size())
		return(1);
	if(arg.size() >= 2 && arg[0] == '-' && arg[1] == '-')
	{
		for(std::vector<t_arg>::iterator it = _args.begin(); it != _args.end(); it++)
		{
			if((*it).longName == arg.substr(2))
			{
				if((*it).isFlag)
					_values[(*it).longName] = "yes";
				else if(*i == argc - 1)
				{
					std::cerr << "missing option for --" << (*it).longName << std::endl;
					return(0);
				}
				else
					_values[(*it).longName] = argv[++(*i)];
				return(1);
			}
		}
		std::cerr<< "unrecognized option : " << arg << std::endl;
		return(0);
	}
	else if(arg[0] == '-')
	{
		for(size_t j = 1; j < arg.size(); j++)
		{
			for(std::vector<t_arg>::iterator it = _args.begin(); it != _args.end(); it++)
			{
				if((*it).shortName == arg[j])
				{
					if((*it).isFlag)
						_values[(*it).longName] = "yes";
					else if(*i == argc - 1)
					{
						std::cerr << "missing option for --" << (*it).longName << std::endl;
						return(0);
					}
					else
						_values[(*it).longName] = argv[++(*i)];
					return(1);
				}
			}
			std::cerr << "unrecognized option : -" << arg[j] << std::endl;
			return(0);
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
