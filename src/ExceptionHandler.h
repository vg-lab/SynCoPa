/*
 Copyright (c) 2026. VG-Lab/URJC.

 Authors: Felix de las Pozas Alvarez <felix.delaspozas@urjc.es>

 This library is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License version 3.0 as published
 by the Free Software Foundation.

 This library is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 details.

 You should have received a copy of the GNU Lesser General Public License
 along with this library; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _EXCEPTION_HANDLER_H_
#define _EXCEPTION_HANDLER_H_

// C++
#include <exception>
#include <source_location>
#include <string>
#include <cstdint>

// Qt
#include <QTextStream>

namespace Utils
{
    /** \class CustomException
     * \brief Implements a exception with source location to use in the application.
     */
    class CustomException : public std::exception, public std::source_location
    {
      public:
        /** \brief CustomException class constructor. 
         * \param[in] message Exception message. 
         * \param[in] location Location where the exception was thrown.
         */
        CustomException(const std::string& message,
                           std::source_location location = std::source_location::current()) :
            std::exception{},
            std::source_location{location},
            m_message{message} {};

        /** \brief CustomException class virtual destructor. 
         */
        virtual ~CustomException() = default;

        const char* what() const noexcept override
        {
            return m_message.c_str();
        }

      private:
        std::string m_message; /** exception message. */
    };

    /** \brief Installs the signal handler and reserves the memory for an
     *         alternate stack for tracing.
     */
    void installSignalHandler();

    /** \brief Installs the handler for unmanaged exceptions.
     */
    void installExceptionHandler();

    /** \brief Helper function to trace the stack and print method names.
     * \param[in] stream text stream where the stack info will be written.
     */
    void backtrace_stack_print(QTextStream& stream);

    extern const int STACK_SIZE;      /** Stack size of stack trace printing. */
    extern uint8_t alternate_stack[]; /** copy of the stack to correctly printing the stack. Only used in Linux */
} // namespace Utils

#endif
