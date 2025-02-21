/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Net/SocketAddressIPv4.h"

#if defined(_WIN32)
#	include <ws2ipdef.h>
#	include <iphlpapi.h>
#endif

#if defined(__APPLE__) && !defined(__IOS__)
#   include <sys/ioctl.h>
#   include <sys/sysctl.h>
#   include <net/if.h>
#   include <netinet/if_ether.h>
#endif

#if defined(__LINUX__) || defined(__RPI__) || defined(__ANDROID__)
#   include <sys/ioctl.h>
#   include <net/if.h>
#   include <netinet/if_ether.h>
#endif

namespace traktor::net
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketAddressIPv4", SocketAddressIPv4, SocketAddress)

SocketAddressIPv4::SocketAddressIPv4()
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = 0;
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const sockaddr_in& sockaddr)
:	m_sockaddr(sockaddr)
{
}

SocketAddressIPv4::SocketAddressIPv4(uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const uint32_t addr, uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(addr);
}

SocketAddressIPv4::SocketAddressIPv4(const uint8_t addr[4], uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(*reinterpret_cast< const uint32_t* >(addr));
}

SocketAddressIPv4::SocketAddressIPv4(const std::wstring& host, uint16_t port)
{
	uint32_t ia = INADDR_NONE;

#if defined(_WIN32)
	if (host == L"localhost")
		ia = htonl(INADDR_LOOPBACK);
	else
#endif
	{
		// Try to resolve address, first try string denoted IP number as it will
		// probably fail faster than gethostbyname.
		ia = inet_addr(wstombs(host).c_str());
		if (ia == INADDR_NONE)
		{
			hostent* hostent = gethostbyname(wstombs(host).c_str());
			if (hostent != 0)
				ia = *reinterpret_cast< uint32_t* >(hostent->h_addr_list[0]);
		}
	}

	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = ia;
}

bool SocketAddressIPv4::valid() const
{
	return bool(m_sockaddr.sin_port != 0);
}

std::wstring SocketAddressIPv4::getHostName() const
{
	return mbstows(inet_ntoa(*const_cast< in_addr* >(&m_sockaddr.sin_addr)));
}

uint32_t SocketAddressIPv4::getAddr() const
{
	return ntohl(m_sockaddr.sin_addr.s_addr);
}

uint16_t SocketAddressIPv4::getPort() const
{
	return ntohs(m_sockaddr.sin_port);
}

void SocketAddressIPv4::setSockAddr(const sockaddr_in& sockaddr)
{
	std::memcpy(&m_sockaddr, &sockaddr, sizeof(sockaddr_in));
}

const sockaddr_in& SocketAddressIPv4::getSockAddr() const
{
	return m_sockaddr;
}

bool SocketAddressIPv4::getInterfaces(std::list< Interface >& outInterfaces)
{
#if defined(_WIN32)
	uint8_t buf[8192];

	ULONG bufLen = 0;
	GetAdaptersInfo(nullptr, &bufLen);
	if (bufLen > sizeof(buf))
		return false;

	if (GetAdaptersInfo((PIP_ADAPTER_INFO)buf, &bufLen) != NO_ERROR)
		return false;

	for (PIP_ADAPTER_INFO ii = (PIP_ADAPTER_INFO)buf; ii; ii = ii->Next)
	{
		Interface itf;

		if (ii->Type == MIB_IF_TYPE_PPP)
			itf.type = ItVPN;
		else if (ii->Type == IF_TYPE_IEEE80211)
			itf.type = ItWiFi;
		else
			itf.type = ItDefault;

		sockaddr_in addr;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = inet_addr(ii->IpAddressList.IpAddress.String);

		if (!addr.sin_addr.s_addr)
			continue;

		itf.addr = new SocketAddressIPv4(addr);

		outInterfaces.push_back(itf);
	}

	sockaddr_in addr;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (addr.sin_addr.s_addr)
	{
		Interface itf;
		itf.type = ItLoopback;
		itf.addr = new SocketAddressIPv4(addr);
		outInterfaces.push_back(itf);
	}

#elif defined(__LINUX__) || defined(__RPI__) || defined(__ANDROID__) || defined(__APPLE__) && !defined(__IOS__)

    uint8_t buf[32767] = { 0 };
    struct ifconf ifc = { 0 };
    struct ifreq* ifr = nullptr;
    int32_t s;

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        log::error << L"Unable to get network interface(s); socket failed" << Endl;
        return false;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (char*)buf;

    if (ioctl(s, SIOCGIFCONF, &ifc) < 0)
    {
        log::error << L"Unable to get network interface(s); ioctl failed" << Endl;
        ::close(s);
        return false;
    }

	uint8_t* ptr = (uint8_t*)ifc.ifc_req;
    for (uint32_t i = 0; i < ifc.ifc_len; )
    {
        struct ifreq& r = *(struct ifreq*)&ptr[i];

#if defined(__LINUX__) || defined(__RPI__) || defined(__ANDROID__)
		i += sizeof(r);
#else
		i += IFNAMSIZ + r.ifr_addr.sa_len;
#endif

		if (ioctl(s, SIOCGIFADDR, &r) < 0)
			continue;

		struct sockaddr& sa = r.ifr_addr;
        if (sa.sa_family == AF_INET)
        {
			Interface n;
			n.type = ItDefault;
			n.addr = new SocketAddressIPv4(*(sockaddr_in*)&sa);

			if (ioctl(s, SIOCGIFFLAGS, &r) >= 0)
			{
				if (r.ifr_flags & IFF_POINTOPOINT)
					n.type = ItVPN;
				if (r.ifr_flags & IFF_LOOPBACK)
					n.type = ItLoopback;
			}
			else
				T_DEBUG(L"Unable to query interface flags; assuming wired interface");

			outInterfaces.push_back(n);
        }
        else
			T_DEBUG(L"Interface " << i << L" not IPv4; skipped");
    }

    ::close(s);

#endif

	return true;
}

bool SocketAddressIPv4::getBestInterface(Interface& outInterface)
{
	std::list< Interface > interfaces;
	if (!getInterfaces(interfaces) || interfaces.empty())
		return false;

	// Prefer wired, default interfaces.
	for (std::list< net::SocketAddressIPv4::Interface >::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
	{
		if (i->type == net::SocketAddressIPv4::ItDefault)
		{
			outInterface = *i;
			return true;
		}
	}

	// Prefer wifi over vpn.
	for (std::list< net::SocketAddressIPv4::Interface >::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
	{
		if (i->type == net::SocketAddressIPv4::ItWiFi)
		{
			outInterface = *i;
			return true;
		}
	}

	// No one left except vpn or loopback.
	outInterface = interfaces.front();
	return true;
}

bool SocketAddressIPv4::operator == (const SocketAddressIPv4& rh) const
{
	return
		m_sockaddr.sin_family == rh.m_sockaddr.sin_family &&
		m_sockaddr.sin_port == rh.m_sockaddr.sin_port &&
		m_sockaddr.sin_addr.s_addr == rh.m_sockaddr.sin_addr.s_addr;
}

}
