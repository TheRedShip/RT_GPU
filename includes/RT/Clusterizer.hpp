/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:25:18 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/25 01:49:07 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

# include "RT.hpp"


typedef enum e_job_status
{
	WAITING,
	IN_PROGRESS,
	DONE
} t_job_status;

typedef struct s_job
{
	glm::vec3	pos;
	glm::vec2	dir;
	size_t		samples;
	GPUDenoise	denoise;

	size_t			frameNb;
} t_job;

typedef struct s_client
{
	std::vector<uint8_t> buffer;
	t_job				*curJob;
	uint8_t				progress;
	bool				ready;
}	t_client;

typedef enum e_msg
{
	RDY,
	JOB,
	PROGRESS_UPDATE,
	IMG_SEND_RQ,
	IMG
} t_msg;

class Clusterizer
{
	public:
		Clusterizer(Arguments &args, Renderer *renderer);
		~Clusterizer();

		void	update(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		bool	getError(void);
		void	imguiRender(void);
		bool	isServer(void);	
		bool	hasJobs(void);

		void	addJob(glm::vec3 pos, glm::vec2 dir, size_t samples, size_t frames, GPUDenoise &denoise);

	private:
		bool				_isActive;
		bool				_isServer;
		bool				_error;
		std::string			_sceneName;
		Renderer			*_renderer;

		std::vector<t_job *> _jobs[3];

		void	imguiJobStat(void);
		void	imguiClients(void);

	private: //client
		void	initClient(std::string &dest);
		void	openClientConnection(const char *ip, int port);
		void	clientHandleBuffer(void);
		void	updateClient(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		void	clientGetJob(void);
		void	clientReceive(void);
		void	handleCurrentJob(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);
		void	sendProgress(uint8_t progress);
		void	sendImageToServer(std::vector<GLuint> &textures, ShaderProgram &denoisingProgram);

		int						_serverFd;
		std::string				_serverIp;
		int						_serverPort;
		std::vector<uint8_t>	_receiveBuffer;
		t_job					*_currentJob;
		uint8_t					_progress;
		bool					_srvReady;

	private: //server
		void	initServer(std::string port);
		void	updateServer(void);

		void	initServerSocket(int port);
		int		acceptClients(void);
		void	updatePollfds(void);
		int		updateBuffer(int fd);
		void	handleBuffer(int fd, std::vector<uint8_t> &buf);
		void	deleteClient(int fd);

		int		dispatchJobs(void);

		int							_serverSocket;
		struct pollfd				*_pollfds;
		std::map<int, t_client>		_clients;
		size_t						_curId;
};
