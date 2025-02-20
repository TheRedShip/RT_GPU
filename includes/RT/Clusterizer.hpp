/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:25:18 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/20 22:41:05 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTERIZER_HPP
# define CLUSTERIZER_HPP

# include "RT.hpp"

typedef struct s_job
{
	std::string	scene;
	glm::vec3	pos;
	glm::vec2	dir;
	size_t		samples;
} t_job;

class Clusterizer
{
	public:
		Clusterizer(Arguments args);
		~Clusterizer();

	private:
		void update(void);
		bool getError(void);

		bool				_isActive;
		bool				_isServer;
		bool				_error;
		std::vector<t_job> _jobs;

	private: //client
		void initClient(void);
		void updateClient(void);

		std::string			_serverIp;

	private: //server
		void initServer(std::string port);
		void updateServer(void);

		void	initServerSocket(uint16_t port);

		int		_serverSocket;




};

#endif
