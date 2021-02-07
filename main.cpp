#include <iostream>
#include <security/pam_appl.h>
#include <string.h>
#include <string_view>

// ==========================================================================================================
// Helpers

std::string_view IntStyleToString(int MessageStyle) {
   switch(MessageStyle) {
      case PAM_PROMPT_ECHO_OFF: return "PAM_PROMPT_ECHO_OFF";
      case PAM_PROMPT_ECHO_ON: return "PAM_PROMPT_ECHO_ON";
      case PAM_ERROR_MSG: return "PAM_ERROR_MSG";
      case PAM_TEXT_INFO: return "PAM_TEXT_INFO";
      default: return "UnexpectedMessageStyle";
   }
}
std::string PamErrorToString(pam_handle_t *Handle, int ErrNum) {
   if(ErrNum == PAM_SYSTEM_ERR)
      return strerror(errno);
   const char *errstr = pam_strerror(Handle, ErrNum);
   if(errstr == nullptr)
      return std::string("PAM error: ") + std::to_string(ErrNum);
   return errstr;
}
void PrintErrAndExit(pam_handle_t *Handle, int ErrNum) {
   std::cout << PamErrorToString(Handle, ErrNum) << std::endl;
   exit(1);
}


// ==========================================================================================================
// Conversation: PAM <-> App: https://man7.org/linux/man-pages/man3/pam_conv.3.html

int Converse(int NumOfMsgs, const pam_message **Messages, pam_response **Reply, [[maybe_unused]] void *This) {
   if(NumOfMsgs <= 0 || NumOfMsgs > PAM_MAX_NUM_MSG)
      exit(2);
   // if(This == nullptr)
   //    exit(2);

   pam_response *Responses = static_cast<pam_response *>(calloc(NumOfMsgs, sizeof(pam_response)));
   if(Responses == nullptr)
      exit(2);
   *Reply = Responses;

   for(int i = 0; i < NumOfMsgs; ++i) {
      const pam_message *Message = Messages[i];
      std::cout << "DEBUG:  Message: " << IntStyleToString(Message->msg_style) << ": " << Message->msg
                << std::endl;
      if(Message->msg_style == PAM_PROMPT_ECHO_OFF || Message->msg_style == PAM_PROMPT_ECHO_ON) {
         std::string UserResponse;
         std::cin >> UserResponse;
         // The resp_retcode member of this struct is unused and should be set to zero.
         Responses[i].resp_retcode = 0;
         Responses[i].resp         = strdup(UserResponse.data());
      }
   }

   // TODO
   // Should be "transactional", in the sense that it:
   // * either sets all pointers, allocates memory AND returns PAM_SUCCESS: from man:
   //   On failure, the conversation function should release any resources it has allocated,
   //   and return one of the predefined PAM error codes.
   // * or, returns PAM_CONV_ERR but does not set any pointers / does not expect caller to free resources.
   // PAM_MAX_RESP_SIZE

   return PAM_SUCCESS;
}

// ==========================================================================================================
// Main

int main(int argc, char **argv) {
   if(argc != 3) {
      std::cout << "Usage: pamtest <service_name> <user_name>" << std::endl;
      exit(5);
   }
   const char *const ServiceName     = argv[1];   // "chrome_pass";
   const char *const User            = argv[2];   // "dimanne";
   pam_conv          PamConversation = {Converse, nullptr /*this*/};
   pam_handle_t *    Handle;
   int               LastRetCode;
   {   // pam_start: https://man7.org/linux/man-pages/man3/pam_start.3.html
      LastRetCode = pam_start(ServiceName, User, &PamConversation, &Handle);
      if(LastRetCode != PAM_SUCCESS)
         PrintErrAndExit(Handle, LastRetCode);
      std::cout << "DEBUG: pam_start done\n" << std::endl;
   }

   {   // pam_authenticate: https://man7.org/linux/man-pages/man3/pam_authenticate.3.html
      /// PAM_DISALLOW_NULL_AUTHTOK - The PAM module service should return PAM_AUTH_ERR if the user does not
      /// have a registered authentication token.
      LastRetCode = pam_authenticate(Handle, PAM_DISALLOW_NULL_AUTHTOK);
      if(LastRetCode != PAM_SUCCESS)
         PrintErrAndExit(Handle, LastRetCode);
      std::cout << "DEBUG: pam_authenticate done\n" << std::endl;
   }

   {   // pam_end: https://man7.org/linux/man-pages/man3/pam_end.3.html
      LastRetCode = pam_end(Handle, LastRetCode);
      Handle      = nullptr;
      if(LastRetCode != PAM_SUCCESS)
         PrintErrAndExit(Handle, LastRetCode);
      std::cout << "DEBUG: pam_end done\n" << std::endl;
   }

   std::cout << "Success!!!" << std::endl;

   return 0;
}
