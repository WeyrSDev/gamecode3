
#pragma once
//========================================================================
// CriticalSection.h : Defines helper classes for multiprogramming
//
// Part of the GameCode3 Application
//
// GameCode3 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 3rd Edition" by Mike McShaffry, published by
// Charles River Media. ISBN-10: 1-58450-680-6   ISBN-13: 978-1-58450-680-5
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the author a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1584506806?ie=UTF8&tag=gamecodecompl-20&linkCode=as2&camp=1789&creative=390957&creativeASIN=1584506806
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: http://gamecode3.googlecode.com/svn/trunk/
//
// (c) Copyright 2009 Michael L. McShaffry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License v2
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================
 
#include <windows.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
 
class CriticalSection : public boost::noncopyable
{
public:
	CriticalSection()
	{ 
		InitializeCriticalSection( &m_cs ); 
	}
 
	~CriticalSection()
	{
		DeleteCriticalSection( &m_cs ); 
	}
 
 
	void Lock()
    { 
		EnterCriticalSection( &m_cs ); 
	}

	void Unlock()
    { 
		LeaveCriticalSection( &m_cs ); 
	}
 
protected:
       // the critical section itself
    mutable CRITICAL_SECTION m_cs;
};
 
 
/*
 Description
      
       helper class
 
 
       allows automatic Locking/ Unlocking of a Resource,
       protected by a Critical Section:
       - locking when this object gets constructed
       - unlocking when this object is destructed
        (goes out of scope)
 
 
 Usage
 
 
       when you need protected access to a resource, do the following
       1. Create a Critical Section associated with the resource
       2. Embody the code accessing the resource in braces {}
       3. Construct an ScopedCriticalSection object
 
 
  Example:
       // we assume that m_CriticalSection
       // is a private variable, and we use it to protect
       // 'this' from being accessed while we need safe access to a resource
      
 
 
       // code that does not need resource locking
      
       {
              ScopedCriticalSection I_am_locked( m_cs);
             
              // code that needs thread locking
       }
      
       // code that does not need resource locking
 
 
*/
class ScopedCriticalSection : public boost::noncopyable
{
public:
	ScopedCriticalSection( CriticalSection & csResource)
		: m_csResource( csResource)
	{
		m_csResource.Lock();
	}
  
	~ScopedCriticalSection()
	{
		m_csResource.Unlock();
	}
 
private:
       // the Critical Section for this resource
       CriticalSection & m_csResource;
};
 
// concurrent_queue was grabbed from 
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// and was written by Anthony Williams
//
// class concurrent_queue					- Chapter 18, page 669
//
template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
public:
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
    }

};

 

