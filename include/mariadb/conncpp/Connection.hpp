/************************************************************************************
   Copyright (C) 2020 MariaDB Corporation AB

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc.,
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/


#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "buildconf.hpp"
#include "SQLString.hpp"
#include "Properties.hpp"
#include "Savepoint.hpp"
#include "jdbccompat.hpp"

namespace sql
{
class Statement;
class PreparedStatement;
class CallableStatement;
class DatabaseMetaData;
class SQLWarning;

enum {
    TRANSACTION_NONE= 0,
    TRANSACTION_READ_UNCOMMITTED= 1,
    TRANSACTION_READ_COMMITTED= 2,
    TRANSACTION_REPEATABLE_READ= 4,
    TRANSACTION_SERIALIZABLE =8
  };

class MARIADB_EXPORTED Connection {
  Connection(const Connection &);
  void operator=(Connection &);
public:
  Connection() {}
  virtual ~Connection(){}

  virtual Statement* createStatement()=0;
  virtual Statement* createStatement(int32_t resultSetType, int32_t resultSetConcurrency)=0;
  virtual Statement* createStatement(int32_t resultSetType, int32_t resultSetConcurrency, int32_t resultSetHoldability)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql, int32_t resultSetType,int32_t resultSetConcurrency)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql, int32_t resultSetType, int32_t resultSetConcurrency, int32_t resultSetHoldability)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql, int32_t autoGeneratedKeys)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql, int32_t* columnIndexes)=0;
  virtual PreparedStatement* prepareStatement(const SQLString& sql, const SQLString* columnNames)=0;
  virtual CallableStatement* prepareCall(const SQLString& sql)=0;
  virtual CallableStatement* prepareCall(const SQLString& sql,int32_t resultSetType,int32_t resultSetConcurrency)=0;
  virtual CallableStatement* prepareCall(const SQLString& sql, int32_t resultSetType, int32_t resultSetConcurrency, int32_t resultSetHoldability)=0;
  virtual SQLString nativeSQL(const SQLString& sql)=0;
  virtual bool getAutoCommit()=0;
  virtual void setAutoCommit(bool autoCommit)=0;
  virtual void commit()=0;
  virtual void rollback()=0;
  virtual void rollback(const Savepoint* savepoint)=0;
  virtual void close()=0;
  virtual bool isClosed()=0;
  virtual DatabaseMetaData* getMetaData()=0;
  virtual bool isReadOnly()=0;
  virtual void setReadOnly(bool readOnly)=0;
  virtual SQLString getCatalog()=0;
  virtual void setCatalog(const SQLString& catalog)=0;
  virtual int32_t getTransactionIsolation()=0;
  virtual void setTransactionIsolation(int32_t level)=0;
  virtual SQLWarning* getWarnings()=0;
  virtual void clearWarnings()=0;
  virtual int32_t getHoldability()=0;
  virtual void setHoldability(int32_t holdability)=0;
  virtual Savepoint* setSavepoint()=0;
  virtual Savepoint* setSavepoint(const SQLString& name)=0;
  virtual void releaseSavepoint(const Savepoint* savepoint)=0;

  virtual bool isValid(int32_t timeout)=0;
  virtual bool isValid()=0;

  virtual void setClientInfo(const SQLString& name,const SQLString& value)=0;
  virtual void setClientInfo(const Properties& properties)=0;
  virtual Properties getClientInfo()=0;
  virtual SQLString getClientInfo(const SQLString& name)=0;

  virtual SQLString getUsername()=0;
  virtual SQLString getHostname()=0;

  virtual int32_t getNetworkTimeout()=0;
  virtual SQLString getSchema()=0;
  virtual void setSchema(const SQLString& arg0)=0;
  virtual void reset()=0;

  virtual bool reconnect()=0;

  virtual Connection* setClientOption(const SQLString& name, void* value)=0;
  virtual Connection* setClientOption(const SQLString& name, const SQLString& value)=0;
  virtual void getClientOption(const SQLString& n, void* v)=0;
  virtual SQLString getClientOption(const SQLString& n)=0;

  virtual Clob* createClob()=0;
  virtual Blob* createBlob()=0;
  virtual NClob* createNClob()=0;
  virtual SQLXML* createSQLXML()=0;
#ifdef JDBC_SPECIFIC_TYPES_IMPLEMENTED
  virtual sql::Array* createArrayOf(const SQLString& typeName, const sql::Object* elements)=0;
  virtual sql::Struct* createStruct(const SQLString& typeName, const sql::Object* attributes)=0;
  virtual void abort(sql::Executor* executor)=0;
  virtual void setNetworkTimeout(Executor* executor, uint32_t milliseconds)=0;
#endif
};
}
#endif
