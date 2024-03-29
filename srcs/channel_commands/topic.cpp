/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcueille <jcueille@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/12 17:46:28 by jcueille          #+#    #+#             */
/*   Updated: 2022/08/24 11:11:28 by jcueille         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/includes.hpp"
#include "../../includes/replies.hpp"

/**
 * @brief	Print topic if topic is empty
 * 			clear channel topic if topic = :
 * 			set channel topic if : + string
 * 
 * @param topic 
 * @param chan target channel name
 * @param u user launching the command
 * @return int 
 */
int TOPIC(const std::string &chan, std::vector<std::string> t,  user *u)
{
	channel *c = find_channel_by_name(chan);
	if (!c)
		return send_message(create_msg(ERR_NEEDMOREPARAMS, u, "TOPIC", "", "", ""), u, ERR_NEEDMOREPARAMS);
	if (!find_u_in_chan(u->nickname, c))
		return send_message(create_msg(ERR_NOTONCHANNEL, u, c->name, "", "", ""), u, ERR_NOTONCHANNEL);
	if (t.empty())
	{
		if (c->topic == "")
			return send_message(create_msg(RPL_NOTOPIC, u, c->name, "", "", ""), u, RPL_NOTOPIC);
		return send_message(create_msg(RPL_TOPIC, u, c->name, c->topic, "", ""), u, RPL_TOPIC);
	}
	if (c->modes[TOPIC_LOCKED_MODE] && !is_chan_ope(c, u))
		return send_message(create_msg(ERR_CHANOPRIVSNEEDED, u, c->name, "", "", ""), u, ERR_CHANOPRIVSNEEDED);
	t.erase(t.begin());
	c->topic = concatenate_vector(t.begin(), t.end());
	std::vector<user *>::iterator it = c->users.begin();
	for (; it != c->users.end(); it++)
		send_message(channel_message("TOPIC " + c->topic, u), (*it), 0);
	return 0;
}