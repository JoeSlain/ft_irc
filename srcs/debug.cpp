/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   debug.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jcueille <jcueille@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/21 12:23:57 by jcueille          #+#    #+#             */
/*   Updated: 2022/06/29 21:40:13 by jcueille         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/includes.hpp"

extern channel* channels;
extern user* users;

void print_channels()
{
	channel *tmp = channels;
	while (tmp)
	{
		std::cout << "channel name: " <<tmp->name << std::endl;
		tmp = tmp->next;
	}
}

void print_str_vec(std::vector<std::string> v, std::string info)
{
	std::vector<std::string>::iterator it = v.begin();
	for (; it != v.end(); it++)
		std::cout << info << (*it) << std::endl;
}

void printer(std::string s)
{
	std::cout << s << std::endl;
}

//pretty printer
void pp(std::string s)
{
	for (size_t i = 0; i < s.size() + 4; i++)
		std::cout << "*";
	std::cout << '\n' << "* " << s << " *" <<std::endl;
	for (size_t i = 0; i < s.size() + 4; i++)
		std::cout << "*";
	std::cout << std::endl;
}

void print_user(user *u)
{
	std::cout << "+++\nUSER:\n" << "nick: " << u->nickname << " hostname: " << u->hostname << " username: " << u->username << " realname: " << u->realname << std::endl;
	std::cout << "MODES: "
	<< (u->modes[AWAY_MODE] ? "away " : "")
	<< (u->modes[INVISIBLE_MODE] ? "invisible " : "")
	<< (u->modes[WALLOPS_MODE] ? "wallops " : "")
	<< (u->modes[RESTRICTED_MODE] ? "restricted " : "")
	<< (u->modes[OPERATOR_MODE] ? "operator " : "")
	<< "+++" << std::endl;
}