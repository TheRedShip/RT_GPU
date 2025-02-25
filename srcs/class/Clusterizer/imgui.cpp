/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   imgui.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tomoron <tomoron@student.42angouleme.fr>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/22 19:52:51 by tomoron           #+#    #+#             */
/*   Updated: 2025/02/24 20:52:33 by tomoron          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RT.hpp"

void Clusterizer::imguiJobStat(void)
{
	ImGui::Text("jobs : ");
	ImGui::Text("  waiting : %lu", _jobs[WAITING].size());
	ImGui::Text("  in progress : %lu", _jobs[IN_PROGRESS].size());
	ImGui::Text("  done : %lu", _jobs[DONE].size());
}

void Clusterizer::imguiClients(void)
{
	std::string status;

	ImGui::Text("clients : ");
	ImGui::BeginChild("clientList", ImVec2(0, 0), true, 0);
	for(auto it = _clients.begin();it != _clients.end(); it++)
	{
		if(it->second.ready)
			status = "idle";
		else if(!it->second.ready && it->second.curJob)
			status = "working";
		else if(it->second.ready)
			status = "not ready";
		
		ImGui::Text("status : %s", status.c_str());
		if(it->second.curJob)
			ImGui::ProgressBar((float)it->second.progress / 100);
		if(std::next(it) != _clients.end())
			ImGui::Separator();
	}
	ImGui::EndChild();
}

void Clusterizer::imguiRender(void)
{
	if(!_isServer)
		return ;
	
	if (ImGui::CollapsingHeader("Clusterizer"))
	{
		imguiJobStat();
		ImGui::Separator();
		imguiClients();
	}
}
