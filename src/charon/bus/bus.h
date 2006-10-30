/**
 * @file bus.h
 *
 * @brief Interface of bus_t.
 *
 */

/*
 * Copyright (C) 2006 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#ifndef BUS_H_
#define BUS_H_

typedef enum signal_t signal_t;
typedef enum level_t level_t;
typedef struct bus_listener_t bus_listener_t;
typedef struct bus_t bus_t;

#include <stdarg.h>

#include <sa/ike_sa.h>
#include <sa/child_sa.h>


/**
 * @brief signals emitted by the daemon.
 *
 * Signaling is for different purporses. First, it allows debugging via
 * "debugging signal messages", sencondly, it allows to follow certain
 * mechanisms currently going on in the daemon. As we are multithreaded, 
 * and of multiple transactions are involved, it's not possible to follow
 * one connection setup without further infrastructure. These infrastructure
 * is provided by the bus and the signals the daemon emits to the bus.
 *
 * There are different scenarios to follow these signals, but all have
 * the same scheme. First, a START signal is emitted to indicate the daemon
 * has started to 
 *
 * @ingroup bus
 */
enum signal_t {
	/** pseudo signal, representing any other signal */
	SIG_ANY,
	
	/** debugging message from daemon main loop */
	DBG_DMN,
	/** debugging message from IKE_SA_MANAGER */
	DBG_MGR,
	/** debugging message from an IKE_SA */
	DBG_IKE,
	/** debugging message from a CHILD_SA */
	DBG_CHD,
	/** debugging message from job processing */
	DBG_JOB,
	/** debugging message from configuration backends */
	DBG_CFG,
	/** debugging message from kernel interface */
	DBG_KNL,
	/** debugging message from networking */
	DBG_NET,
	/** debugging message from message encoding/decoding */
	DBG_ENC,
	/** debugging message from libstrongswan via logging hook */
	DBG_LIB,
	
	/** number of debug signals */
	DBG_MAX,
	
	/** signals for IKE_SA establishment */
	IKE_UP_START,
	IKE_UP_SUCCESS,
	IKE_UP_FAILED,
	
	/** signals for IKE_SA delete */
	IKE_DOWN_START,
	IKE_DOWN_SUCCESS,
	IKE_DOWN_FAILED,
	
	/** signals for IKE_SA rekeying */
	IKE_REKEY_START,
	IKE_REKEY_SUCCESS,
	IKE_REKEY_FAILED,
	
	/** signals for CHILD_SA establishment */
	CHILD_UP_START,
	CHILD_UP_SUCCESS,
	CHILD_UP_FAILED,
	
	/** signals for CHILD_SA delete */
	CHILD_DOWN_START,
	CHILD_DOWN_SUCCESS,
	CHILD_DOWN_FAILED,
	
	/** signals for CHILD_SA rekeying */
	CHILD_REKEY_START,
	CHILD_REKEY_SUCCESS,
	CHILD_REKEY_FAILED,
	
	/** signals for CHILD_SA routing */
	CHILD_ROUTE_START,
	CHILD_ROUTE_SUCCESS,
	CHILD_ROUTE_FAILED,
	
	/** signals for CHILD_SA routing */
	CHILD_UNROUTE_START,
	CHILD_UNROUTE_SUCCESS,
	CHILD_UNROUTE_FAILED,
	
	SIG_MAX
};

/**
 * short names of signals using 3 chars
 */
extern enum_name_t *signal_names;

/**
 * Signal levels used to control output verbosity.
 */
enum level_t {
	/** numerical levels from 0 to 4 */
	LEVEL_0 = 0,
	LEVEL_1 = 1,
	LEVEL_2 = 2,
	LEVEL_3 = 3,
	LEVEL_4 = 4,
	/** absolutely silent, no signal is emitted with this level */
	LEVEL_SILENT = -1,
	/** alias for numberical levels */
	LEVEL_AUDIT = LEVEL_0,
	LEVEL_CTRL = LEVEL_1,
	LEVEL_CTRLMORE = LEVEL_2,
	LEVEL_RAW = LEVEL_3,
	LEVEL_PRIVATE = LEVEL_4,
};

/**
 * @brief Raise a signal for an occured event.
 *
 * @param sig		signal_t signal description
 * @param format	printf() style format string
 * @param ...		printf() style agument list
 */
#define SIG(sig, format, ...) charon->bus->signal(charon->bus, sig, LEVEL_0, format, ##__VA_ARGS__)

/**
 * @brief Log a debug message via the signal bus.
 *
 * @param signal	signal_t signal description
 * @param format	printf() style format string
 * @param ...		printf() style agument list
 */
#define DBG1(sig, format, ...) charon->bus->signal(charon->bus, sig, LEVEL_1, format, ##__VA_ARGS__)
#define DBG2(sig, format, ...) charon->bus->signal(charon->bus, sig, LEVEL_2, format, ##__VA_ARGS__)
#define DBG3(sig, format, ...) charon->bus->signal(charon->bus, sig, LEVEL_3, format, ##__VA_ARGS__)
#define DBG4(sig, format, ...) charon->bus->signal(charon->bus, sig, LEVEL_4, format, ##__VA_ARGS__)

/**
 * @brief Get the type of a signal.
 *
 * A signal may be a debugging signal with a specific context. They have
 * a level specific for their context > 0. All audit signals use the
 * type 0. This allows filtering of singals by their type.
 *
 * @param signal	signal to get the type from
 * @return			type of the signal, between 0..(DBG_MAX-1)
 */
#define SIG_TYPE(sig) (sig > DBG_MAX ? SIG_ANY : sig)


/**
 * @brief Interface for registering at the signal bus.
 *
 * To receive signals from the bus, the client implementing the
 * bus_listener_t interface registers itself at the signal bus.
 *
 * @ingroup bus
 */
struct bus_listener_t {
	
	/**
	 * @brief Send a signal to a bus listener.
	 *
	 * A numerical identification for the thread is included, as the
	 * associated IKE_SA, if any. Signal specifies the type of
	 * the event occured. The format string specifies
	 * an additional informational or error message with a printf() like
	 * variable argument list. This is in the va_list form, as forwarding
	 * a "..." parameters to functions is not (cleanly) possible.
	 *
	 * @param this		listener
	 * @param singal	kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param thread	ID of the thread raised this signal
	 * @param ike_sa	IKE_SA associated to the event
	 * @param format	printf() style format string
	 * @param args		vprintf() style va_list argument list
	 */
	void (*signal) (bus_listener_t *this, signal_t signal, level_t level,
					int thread, ike_sa_t *ike_sa, char* format, va_list args);
};

/**
 * @brief Signal bus which sends signals to registered listeners.
 *
 * The signal bus is not much more than a multiplexer. A listener interested
 * in receiving event signals registers at the bus. Any signals sent to
 * are delivered to all registered listeners.
 * To deliver signals to threads, the blocking listen() call may be used
 * to wait for a signal.
 *
 * @ingroup bus
 */
struct bus_t {
	
	/**
	 * @brief Register a listener to the bus.
	 *
	 * A registered listener receives all signals which are sent to the bus.
	 * The listener is passive; the thread which emitted the signal
	 * processes the listener routine.
	 *
	 * @param this		bus
	 * @param listener	listener to register.
	 */
	void (*add_listener) (bus_t *this, bus_listener_t *listener);
	
	/**
	 * @brief Listen actively on the bus.
	 *
	 * As we are fully multithreaded, we must provide a mechanism
	 * for active threads to listen to the bus. With the listen() method,
	 * a thread waits until a signal occurs, and then processes it.
	 * To prevent the listen() calling thread to miss signals emitted while
	 * it processes a signal, registration is required. This is done through
	 * the set_listen_state() method, see below.
	 *
	 * @param this		bus
	 * @param level		verbosity level of the signal
	 * @param thread	receives thread number emitted the signal
	 * @param ike_sa	receives the IKE_SA involved in the signal, or NULL
	 * @param format	receives the format string supplied with the signal
	 * @param va_list	receives the variable argument list for format
	 * @return			the emitted signal type
	 */
	signal_t (*listen) (bus_t *this, level_t* level, int *thread,
						ike_sa_t **ike_sa, char** format, va_list* args);
	
	/**
	 * @brief Set the listening state of the calling thread.
	 *
	 * To prevent message loss for active listeners using listen(), threads
	 * must register themself to the bus before starting to listen(). When
	 * a signal occurs, the emitter waits until all threads with listen_state
	 * TRUE are waiting in the listen() method to process the signal.
	 * It is important that a thread with liste_state TRUE calls listen()
	 * periodically, or sets it's listening state to FALSE; otherwise
	 * all signal emitting threads get blocked on the bus.
	 *
	 * @param this		bus
	 * @param active	TRUE to set to listening
	 */
	void (*set_listen_state) (bus_t *this, bool active);
	
	/**
	 * @brief Set the IKE_SA the calling thread is using.
	 *
	 * To associate an received signal to an IKE_SA without passing it as
	 * parameter each time, the thread registers it's used IKE_SA each
	 * time it checked it out. Before checking it in, the thread unregisters
	 * the IKE_SA (by passing NULL). This IKE_SA is stored per-thread, so each
	 * thread has one IKE_SA registered (or not).
	 * 
	 * @param this		bus
	 * @param ike_sa	ike_sa to register, or NULL to unregister
	 */
	void (*set_sa) (bus_t *this, ike_sa_t *ike_sa);
	
	/**
	 * @brief Send a signal to the bus.
	 *
	 * The signal specifies the type of the event occured. The format string
	 * specifies an additional informational or error message with a
	 * printf() like variable argument list.
	 * Some useful macros are available to shorten this call.
	 * @see SIG(), DBG1()
	 *
	 * @param this		bus
	 * @param singal	kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param format	printf() style format string
	 * @param ...		printf() style argument list
	 */
	void (*signal) (bus_t *this, signal_t signal, level_t level, char* format, ...);
	
	/**
	 * @brief Send a signal to the bus using va_list arguments.
	 *
	 * Same as bus_t.signal(), but uses va_list argument list.
	 *
	 * @param this		bus
	 * @param singal	kind of the signal (up, down, rekeyed, ...)
	 * @param level		verbosity level of the signal
	 * @param format	printf() style format string
	 * @param args		va_list arguments
	 */
	void (*vsignal) (bus_t *this, signal_t signal, level_t level, char* format, va_list args);
	
	/**
	 * @brief Destroy the signal bus.
	 *
	 * @param this		bus to destroy
	 */
	void (*destroy) (bus_t *this);
};

/**
 * @brief Create the signal bus which multiplexes signals to its listeners.
 *
 * @return		signal bus instance
 * 
 * @ingroup bus
 */
bus_t *bus_create();

#endif /* BUS_H_ */
