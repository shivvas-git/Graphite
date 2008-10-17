#include "syscall_model.h"
#include "sys/syscall.h"
#include "chip.h"
#include "transport.h"

bool called_enter = false;
int ret_val = 0;

SyscallMdl::SyscallMdl(Network *net)
   :the_network(net)
{
}

void SyscallMdl::runExit(int rank, CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   //if only the code below worked in enter...
   //int return_addr = PIN_GetContextReg(ctx, REG_INST_PTR);
   //return_addr += 2;
   //PIN_SetContextReg(ctx, REG_INST_PTR, return_addr);
   //PIN_ExecuteAt(ctx);

   if(called_enter)
   {
      PIN_SetContextReg(ctx, REG_EAX, ret_val);
      called_enter = false;
   }
}

void SyscallMdl::runEnter(int commid, CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   // Reset the buffers for the new transmission
   recv_buff.clear(); 
   send_buff.clear(); 
   
   int msg_type = MCP_MESSAGE_SYS_CALL;
   UInt8 syscall_number = (UInt8) PIN_GetSyscallNumber(ctx, syscall_standard);
   
   send_buff << msg_type << commid << syscall_number;   

   switch(syscall_number)
   {
      case SYS_open:
      {
         char *path = (char *)PIN_GetSyscallArgument(ctx, syscall_standard, 0);
         if(!strcmp(path,"./common/tests/file_io/input"))
         {
            called_enter = true;
            ret_val = marshallOpenCall(ctx, syscall_standard);
         }
         break;
      }
      case SYS_read:
      {
         int fd = PIN_GetSyscallArgument(ctx, syscall_standard, 0);
         if ( fd == 0x03 )
         {
            called_enter = true;
            ret_val = marshallReadCall(ctx, syscall_standard);
         }
         break;
      }

      case SYS_write:
      {
         int fd = PIN_GetSyscallArgument(ctx, syscall_standard, 0);
         if ( fd == 0x03 )
         {
            called_enter = true;
            ret_val = marshallWriteCall(ctx, syscall_standard);
         }         
         break;
      }         
      case SYS_close:
      {
         int fd = PIN_GetSyscallArgument(ctx, syscall_standard, 0);
         if ( fd == 0x03 )
         {
            called_enter = true;
            ret_val = marshallCloseCall(ctx, syscall_standard);
         }
         break;
      }
      case SYS_access:
         called_enter = true;
         ret_val = marshallAccessCall(ctx, syscall_standard);
         break;

      // case SYS_exit:
      //    cerr << "exit()" << endl;
      //    break;
      case -1:
         break;
      default:
         //         cerr << "SysCall: " << (int)syscall_number << endl;
         break;
   }

   if(called_enter)
      PIN_SetSyscallNumber(ctx, syscall_standard, SYS_getpid);


}


int SyscallMdl::marshallOpenCall(CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   /*
       Syscall Args
       const char *pathname, int flags

 
       Transmit Protocol

       Field               Type
       -----------------|--------
       LEN_FNAME           UInt32
       FILE_NAME           char[]
       STATUS_FLAGS        int

       Receive Protocol
       
       Field               Type
       -----------------|--------
       STATUS              int       

   */

   cerr << "Entering SyscallMdl::marshallOpen()" << endl;

   char *path = (char *) PIN_GetSyscallArgument(ctx, syscall_standard, 0);
   int flags = (int) PIN_GetSyscallArgument(ctx, syscall_standard, 1);
   UInt32 len_fname = strlen(path) + 1;

   cerr << "open(" << path << ")" << endl;

   send_buff << len_fname << make_pair(path, len_fname) << flags;
   the_network->getTransport()->ptSendToMCP((UInt8 *) send_buff.getBuffer(), send_buff.size());

   UInt32 length = 0;
   UInt8 *res_buff = the_network->getTransport()->ptRecvFromMCP(&length);
   assert( length == sizeof(int) );
   recv_buff << make_pair(res_buff, length);

   int status;
   recv_buff >> status;

   return status;
}


int SyscallMdl::marshallReadCall(CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{

   /*
       Syscall Args
       int fd, void *buf, size_t count


       Transmit

       Field               Type
       -----------------|--------
       FILE_DESCRIPTOR     int
       COUNT               size_t

       Receive
       
       Field               Type
       -----------------|--------
       BYTES               int
       BUFFER              void *       

   */

   cerr << "Entering syscall model marshall read" << endl;

   int fd = (int) PIN_GetSyscallArgument(ctx, syscall_standard, 0);
   void *buf = (void *) PIN_GetSyscallArgument(ctx, syscall_standard, 1);
   size_t count = (size_t) PIN_GetSyscallArgument(ctx, syscall_standard, 2);

   cerr << "read(" << fd << hex << ", " << buf << dec << ", " << count << ")" << endl;
      
   send_buff << fd << count;
   the_network->getTransport()->ptSendToMCP((UInt8 *) send_buff.getBuffer(), send_buff.size());   
   
   cerr << "sent to mcp " << send_buff.size() << " bytes" << endl;

   UInt32 length = 0;
   UInt8 *res_buff = the_network->getTransport()->ptRecvFromMCP(&length);
   cerr << "received from mcp" << endl;

   assert( length >= sizeof(int) );
   recv_buff << make_pair(res_buff, length);

   int bytes;
   recv_buff >> bytes;

   if ( bytes != -1 )
   {
      recv_buff >> make_pair(buf, bytes);
   } 
   else 
   {
      assert( recv_buff.size() == 0 );
   }
   cerr << "Exiting syscall model marshall read" << endl;

   return bytes;
}


int SyscallMdl::marshallWriteCall(CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   /*
       Syscall Args
       int fd, void *buf, size_t count


       Transmit

       Field               Type
       -----------------|--------
       FILE_DESCRIPTOR     int
       COUNT               size_t
       BUFFER              char[]

       Receive
       
       Field               Type
       -----------------|--------
       BYTES               int       

   */

   cerr << "Entering syscall model marshall write" << endl;

   int fd = (int) PIN_GetSyscallArgument(ctx, syscall_standard, 0);
   void *buf = (void *) PIN_GetSyscallArgument(ctx, syscall_standard, 1);
   size_t count = (size_t) PIN_GetSyscallArgument(ctx, syscall_standard, 2);

   cerr << "write(" << fd << hex << ", " << buf << dec << ", " << count << ")" << endl;
      
   send_buff << fd << count << make_pair(buf, count);
   the_network->getTransport()->ptSendToMCP((UInt8 *) send_buff.getBuffer(), send_buff.size());      

   UInt32 length = 0;
   UInt8 *res_buff = the_network->getTransport()->ptRecvFromMCP(&length);
   assert( length == sizeof(int) );
   recv_buff << make_pair(res_buff, length);

   int status;
   recv_buff >> status;

   return status;
}

int SyscallMdl::marshallCloseCall(CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   /*
       Syscall Args
       int fd, void *buf, size_t count


       Transmit

       Field               Type
       -----------------|--------
       FILE_DESCRIPTOR     int

       Receive
       
       Field               Type
       -----------------|--------
       STATUS              int       

   */

   cerr << "Entering syscall model marshall close" << endl;

   int fd = (int) PIN_GetSyscallArgument(ctx, syscall_standard, 0);      
   send_buff << fd;
   the_network->getTransport()->ptSendToMCP((UInt8 *) send_buff.getBuffer(), send_buff.size());      

   UInt32 length = 0;
   UInt8 *res_buff = the_network->getTransport()->ptRecvFromMCP(&length);
   assert( length == sizeof(int) );
   recv_buff << make_pair(res_buff, length);

   int status;
   recv_buff >> status;

   return status;
}

int SyscallMdl::marshallAccessCall(CONTEXT *ctx, SYSCALL_STANDARD syscall_standard)
{
   cerr << "Entering SyscallMdl::marshallAccessCall()" << endl;

   char *path = (char *)PIN_GetSyscallArgument(ctx, syscall_standard, 0);
   int mode = (int)PIN_GetSyscallArgument(ctx, syscall_standard, 1);
   UInt32 len_fname = strlen(path) + 1;

   // pack the data
   send_buff << make_pair(path, len_fname) << mode;

   // send the data
   the_network->getTransport()->ptSendToMCP((UInt8 *) send_buff.getBuffer(), send_buff.size()); 

   // get a result
   UInt32 length;
   UInt8 *res_buff = the_network->getTransport()->ptRecvFromMCP(&length);

   // Create a buffer out of the result
   recv_buff << make_pair(res_buff, length);

   // return the result
   int result;
   recv_buff >> result;
   return result;
}


