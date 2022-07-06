/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcueille <jcueille@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/30 16:46:15 by jcueille          #+#    #+#             */
/*   Updated: 2022/07/07 00:03:07 by jcueille         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/includes.hpp"
#include "../includes/replies.hpp"

static int USR_MODE(const std::string &target, const char &sign, const char &mode, user *u)
{
	short local_sign;

	if (!find_user_by_nickname(target))
		return send_message(create_msg(ERR_NOSUCHNICK, u, target, "", "", ""), u, ERR_NOSUCHNICK);
	if (target != u->nickname)
		return send_message(create_msg(ERR_USERSDONTMATCH, u, "", "", "", ""), u, ERR_USERSDONTMATCH);
	if (sign)
	{
		if (sign != '+' && sign != '-')
			return send_message(create_msg(ERR_UMODEUNKNOWNFLAG, u, "", "", "", ""), u, ERR_UMODEUNKNOWNFLAG); 
		sign == '+' ? local_sign = 1 : local_sign = 0;
		switch (mode)
		{
		case 'i' :
			u->modes[INVISIBLE_MODE] = local_sign;
			break;
		case 'w' :
			u->modes[WALLOPS_MODE] = local_sign;
			break;
		case 'r':
			if (local_sign)
				return 0;
			u->modes[RESTRICTED_MODE] = local_sign;
			break;
		case 'o':
			if (local_sign)
				return 0;
			u->modes[OPERATOR_MODE] = local_sign;
			break;
		
		default:
			return send_message("ERR_UMODEUNKNOWNFLAG", u, ERR_UMODEUNKNOWNFLAG);
			break;
		}	
	}
	else 
	{
		std::string str;
		
		if (u->modes[INVISIBLE_MODE])
			str += "i";
		if (u->modes[WALLOPS_MODE])
			str += "w";
		if(u->modes[RESTRICTED_MODE])
			str += "r";
		if(u->modes[OPERATOR_MODE])
			str += "o";
		return send_message(create_msg(RPL_UMODEIS, u, str, "", "", ""), u, RPL_UMODEIS);
	}
	return 0;
}

static int CHAN_MODE(const std::string &target, const char &sign, const char &mode, user *u)
{
	channel *c = find_channel_by_name(target);
	short local_sign;

	if (!c)
		return send_message(create_msg(ERR_NOSUCHCHANNEL, u, target, "", "", ""), u, ERR_NOSUCHCHANNEL);
	if (!sign)
	{
		if (!is_chan_ope(c, u))
			return send_message(create_msg(ERR_CHANOPRIVSNEEDED, u, target, "", "", ""), u, ERR_CHANOPRIVSNEEDED);
		if (sign != '+' && sign != '-')
			return send_message(create_msg(ERR_UMODEUNKNOWNFLAG, u, "", "", "", ""), u, ERR_UMODEUNKNOWNFLAG); 
		sign == '+' ? local_sign = 1 : local_sign = 0;
		switch (mode)
		{
		case 'i' :
			c->modes[INVITE_ONLY_MODE] = local_sign;
			break;
		case 'b' :
			c->modes[BAN_MODE] = local_sign;
			break;
		case 'm':
			c->modes[RESTRICTED_MODE] = local_sign;
			break;
		case 'o':
			c->modes[OPERATOR_MODE] = local_sign;
			break;
		
		default:
			return send_message("ERR_UMODEUNKNOWNFLAG", u, ERR_UMODEUNKNOWNFLAG);
			break;
		}
	}
	else
	{
		std::string str;
		if (c->modes[INVITE_ONLY_MODE])
			str += "i";
		if (c->modes[BAN_MODE])
			str += "b";
		if (c->modes[MODERATED_MODE])
			str += "m";
		if (c->modes[NO_EXTERN_MSG_MODE])
			str += "n";
		if (c->modes[SECRET_MODE])
			str += "s";
		if (c->modes[TOPIC_LOCKED_MODE])
			str += "t";
		if (c->modes[KEY_LOCKED_MODE])
			str += "k";
		if (c->modes[USER_LIMIT_MODE])
			str += "l";
	}
	return 0;
}

/**
 * * @brief Changes user modes
 * 
 * @param nickname the target's nickname
 * @param sign + or -
 * @param mode a, i, w, r, o
 * @param u 
 * @return 0 on success 
 */
int MODE(const std::string &target, const std::string &mode, user *u)
{
	pp("target : " + target + "mode :" + mode, RED);
	if (target.empty())
		return send_message(create_msg(ERR_NEEDMOREPARAMS, u, "MODE", "", "", ""), u, ERR_NEEDMOREPARAMS);
	
	if (target[0] == '#' || target[0] == '&')
		return CHAN_MODE(target, mode[0], mode[1],u);
	else
		return USR_MODE(target, mode[0], mode[1], u);
	return 0;
}