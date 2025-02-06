/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Arguments.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 01:07:08 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/04 17:04:59 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARGUMENTS_HPP
# define ARGUMENTS_HPP

#include "RT.hpp"

typedef struct s_arg
{
	char		shortName;
	std::string	longName;
	int			isFlag;
}	t_arg;

class Arguments
{
	public : 
		Arguments(int argc, char **argv);

		bool		getHeadless(void) const;
		bool		error(void) const;
		void		show(void);

		std::string	&getSceneName(void);
		std::string *getRenderPath(void);
		bool		getHeadless(void);

	private:
		void printUsage();

		int		handleArg(char **argv, int argc, int *i);
		void	initArguments(void);
		void	addArgument(char shortName, std::string longName, int isFlag);

		void	parseRenderPathName(char * path);

		bool _headless;
		bool _err;

		std::vector<t_arg> _args;
		std::map<std::string, std::string> _values;
};

#endif
