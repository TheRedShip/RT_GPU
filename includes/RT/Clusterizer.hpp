/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:25:18 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/22 01:43:51 by tomoron          ###   ########.fr       */
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
	size_t		id;
} t_job;

typedef struct s_client
{
	std::vector<uint8_t> buffer;
	t_job				*curJob;
	bool				ready;
}	t_client;

typedef enum e_msg
{
	RDY,
	JOB,
	JOB_RES_RQ,
	ACK,
	WAIT,
	IMAGE,
	ERR,
	UNKNOWN
} t_msg;

class Clusterizer
{
	public:
		Clusterizer(Arguments &args);
		~Clusterizer();

		void update(void);
		bool getError(void);
	private:
		bool				_isActive;
		bool				_isServer;
		bool				_error;

		std::vector<t_job> _jobs;

	private: //client
		void initClient(std::string &dest);
		void openClientConnection(const char *ip, int port);
		void clientHandleBuffer(void);
		void updateClient(void);
		void clientGetJob(std::vector<uint8_t> &sendBuf);
		void clientReceive(void);

		int						_serverFd;
		std::string				_serverIp;
		int						_serverPort;
		std::vector<uint8_t>	_receiveBuffer;
		t_job					_currentJob;

	private: //server
		void initServer(std::string port);
		void updateServer(void);

		void	initServerSocket(int port);
		void	acceptClients(void);
		void	updatePollfds(void);
		int		updateBuffer(int fd);
		void	handleBuffer(int fd, std::vector<uint8_t> &buf);
		void	deleteClient(int fd);
		

		int							_serverSocket;
		struct pollfd				*_pollfds;
		std::map<int, t_client>		_clients;
};

#endif
