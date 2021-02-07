
### How to test

##### Create PAM config for chrome based on [user OS login password](https://man7.org/linux/man-pages/man8/pam_unix.8.html):
```
echo "auth  requisite  pam_unix.so nullok" > /etc/pam.d/chrome_pass
```

##### Compile

```
git clone git@github.com:DimanNe/pam.git && mkdir build-pam && cd build-pam
cmake ../pam/ && make -j
```

##### Test it:

```
./pamtest chrome_pass `whoami`  # OR ./pamtest chrome_pass (whoami) - for fish shell
DEBUG: pam_start done

DEBUG:  Message: PAM_PROMPT_ECHO_OFF: Password: 
test                                   # <<<<<< my password
DEBUG: pam_authenticate done

DEBUG: pam_end done

Success!!!

```
