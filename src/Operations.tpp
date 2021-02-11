/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Operations.tpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:58:49 by nathan            #+#    #+#             */
/*   Updated: 2021/02/11 05:17:12 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//reusable instructions
template<typename T>
void load(T& destination, const T& source)
{
	destination = source;
}
