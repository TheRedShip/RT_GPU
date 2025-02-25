/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clusterizer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 18:25:18 by tomoron           #+#    #+#             */
/*   Updated: 2025/03/18 17:04:13 by tomoron          ###   ########.fr       */
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
	bool				readyRespond;
	bool				gotGo;
}	t_client;

typedef enum e_msg
{
	RDY,
	JOB,
	PROGRESS_UPDATE,
	IMG_SEND_RQ,
	IMG,
	SET_MAP,
	ABORT
} t_msg;

class Clusterizer
{
	public:
		Clusterizer(Arguments &args, Renderer *renderer);
		~Clusterizer();

		void	update(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers);
		bool	getError(void);
		void	imguiRender(void);
		bool	isServer(void);	
		bool	hasJobs(void);

		void	addJob(glm::vec3 pos, glm::vec2 dir, size_t samples, size_t frames, GPUDenoise &denoise);
		void	abortJobs(void);

	private:
		bool				_isActive;
		bool				_isServer;
		bool				_error;
		std::string			_sceneName;
		Renderer			*_renderer;


		void		imguiJobStat(void);
		void		imguiClients(void);
		std::string	clientStatus(t_client client);

	private: //client
		void					initClient(std::string &dest);
		void					openClientConnection(const char *ip, int port);
		void					clientHandleBuffer(Scene &scene, std::vector<Buffer *> &buffers, Window &win);
		void					updateClient(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers);
		void					clientGetJob(void);
		void					clientReceive(Scene &scene, std::vector<Buffer *> &buffers, Window &win);
		void					handleCurrentJob(Scene &scene, Window &win, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers);
		void					sendProgress(uint8_t progress);
		void					sendImageToServer(Scene &scene, std::vector<GLuint> &textures, ShaderProgram &denoisingProgram, std::vector<Buffer *> &buffers, Window &win);
		std::vector<uint8_t>	rgb32fToRgb24i(std::vector<float> &imageFloat);

		void					changeMap(Scene &scene, std::vector<Buffer *> &buffers, Window &win);
		bool					stringComplete(void);

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
		bool	handleBuffer(int fd, std::vector<uint8_t> &buf);
		void	deleteClient(int fd);
		int		handleSendRequests(void);
		void	getImageFromClient(int fd, std::vector<uint8_t> &buf);
		void	redistributeJob(t_job *job);

		int		dispatchJobs(void);

		int							_serverSocket;
		struct pollfd				*_pollfds;
		std::map<int, t_client>		_clients;
		size_t						_curFrame;

		std::vector<t_job *>		_jobs[3];
};
