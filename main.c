
#include "main.h"

static int device_fd=-1;
static int dev_serio_id=-1;
static int print_message=0;
static int print_progress=0;
static int extended_ps2_exercise;

static int m_LittleEndia = 1;
static int support_warpMode;
static int into_wrap_data;
static int poll_time_out = POLL_TIMEOUT;
static FLASH_HANDLER   m_FlashHandle;
static int file_fd;

static char *firmware_binaryA = "elan_pst_rankA.bin";	/* firmware blob */
static char *firmware_binaryB = "elan_pst_rankB.bin";	/* firmware blob */
static char *firmware_binaryC = "elan_pst_rankC.bin";	/* firmware blob */
static int serio_num=-1;
/* Command line parsing related */
static char *progname;
static char *short_opts = ":a:b:c:s:gdmpuPz";
static const struct option long_opts[] = {
    /* name    hasarg *flag val */
{"rank A bin",      1,   NULL, 'a'},
{"rank B bin",      1,   NULL, 'b'},
{"rank C bin",      1,   NULL, 'c'},
{"update firmware",      0,   NULL, 'u'},
{"serionum",   1,   NULL, 's'},
{"get_current_version",    0,   NULL, 'g'},
{"get_module_id",    0,   NULL, 'm'},
{"help",     0,   NULL, '?'},
{"message",    0,   NULL, 'p'},
{"progress",    0,   NULL, 'P'},
{"version",    0,   NULL, 'z'},
{"debug",    0,   NULL, 'd'},
{NULL,       0,   NULL, 0},
};

static void usage(int errs)
{
    printf("\nUsage: %s [options]\n"
           "\n"
           "Firmware updater for elan standalone trackpoint V%s\n"
           "\n"
           "Options:\n"
           "\n"
           "  -a,--bin    STR          	Firmware binary (default %s)\n"
           "  -b,--bin    STR          	Firmware binary (default %s)\n"
           "  -c,--bin    STR          	Firmware binary (default %s)\n"
           "  -s,--serionum INT      	/sys/bus/serio/devices/serioX\n"
           "  -g,--get_current_version      Get Firmware Version\n"
           "  -m,--get_module_id  		Get Module ID\n"
           "  -u,--update firmware      	Update Firmware\n"
           "  -p,--print message            Print Message\n"
           "  -P,--print progress           Print Progress\n"
	   "  -z,--version              	Version\n"
           "  -d,--debug               	Debug ps2 Message\n"
           "  -?,--help               	Show this message\n"
           "\n", progname, VERSION, firmware_binaryA, firmware_binaryB, firmware_binaryC);

    printf("Example:\n"
           "Update firmware      : ./epstps2_updater -a rankA.bin -b rankB.bin -c rankC.bin -u\n"
           "Get Firmware Version : ./epstps2_updater -g\n"
           "Get Module ID        : ./epstps2_updater -m\n");
    exit(!!errs);
}

static int parse_cmdline(int argc, char *argv[])
{
    char *e = 0;
    int i, errorcnt = 0;
    int state = -1;
    progname = strrchr(argv[0], '/');
    if (progname)
        progname++;
    else
        progname = argv[0];

    opterr = 0;				/* quiet, you */
    while ((i = getopt_long(argc, argv, short_opts, long_opts, 0)) != -1) {
        switch (i) {
        case 'a':
            firmware_binaryA = optarg;
            break;
        case 'b':
            firmware_binaryB = optarg;
            break;
        case 'c':
            firmware_binaryC = optarg;
            break;
        case 's':
            serio_num = (uint16_t) strtoul(optarg, &e, 16);
            if (!*optarg || (e && *e)) {
                printf("Invalid argument: \"%s\"\n", optarg);
                errorcnt++;
            }
            break;

        case 'g':
            state = GET_FWVER_STATE;
            break;
        case 'm':
            state = GET_MODULEID_STATE;
            break;
        case 'u':
            state = IAP_STATE;
            break;
        case 'p':
            print_message = 1;
            break;
        case 'P':
            print_progress = 1;
            break;
	case 'z':			
	    state = GET_SWVER_STATE;
	    break;
        case 'd':
            extended_ps2_exercise = 1;
            break;
        case '?':
            if (optopt)
                printf("Unrecognized option: -%c\n", optopt);
            else
                printf("Unrecognized option: %s\n",
                       argv[optind - 1]);
            errorcnt++;
            break;
        case ':':
            printf("Missing argument to %s\n", argv[optind - 1]);
            errorcnt++;
            break;
        default:
            printf("Internal error at %s:%d\n", __FILE__, __LINE__);
            exit(1);
        }
    }

    if (errorcnt)
        usage(errorcnt);
    return state;

}
static void output_message(const char *text)
{
    if(print_message){
        printf("%s\n", text);
    }
}
static int create_serioraw_node()
{

    DIR* FD;
    struct dirent* in_file;
    char pch[256];
    int count=-1;
    int start_num=0;
    int end_num=9;
    FILE  *entry_file;
    if (NULL == (FD = opendir (PSMOUSE_DRIVER_PATH)))  {
        if(print_message)
            printf("Create_SerioRaw_Node : Open %s Error \n",PSMOUSE_DRIVER_PATH);
        return DRIVER_CANNOT_OPENDIR;
    }
    while ((in_file = readdir(FD)))
    {
        if(strstr(in_file->d_name, "serio") == NULL) {
            continue;
        }
        char buffer[2];
        if(serio_num!=-1)
        {
            start_num=serio_num;
            end_num=serio_num+1;
        }
        for(int i=start_num; i<end_num ; i++)
        {
            sprintf(buffer,"%d",i);
            if(strstr(in_file->d_name, buffer ) == NULL) {
                continue;
            }
            //START Issue: Can't find driver node in first time  V1.2 JINGLE_WU 2017/08/24

            char pch[256];
            char str [80];
            sprintf(pch,"%sserio%d/description",PSMOUSE_DRIVER_PATH,i);
            entry_file = fopen(pch, "r");
            // fscanf (entry_file, "%s", str);
            fgets(str, sizeof(str), entry_file) ;

            if(print_message)
                printf("serach node1 : %s  description = %s %s\n", in_file->d_name, str,strstr (str,"i8042") );

            //printf("%s \n", str);
            if((strstr (str,"i8042") != NULL) && (strstr (str,"AUX") != NULL))
            {
                count=i;

                //printf("match count %d \n", i);
                goto create_node;
            }
            fclose(entry_file);

            //END Issue: Can't find driver node in first time  V1.2 JINGLE_WU 2017/08/24
        }

        if(print_message)
            printf("serach node1 : %s  \n", in_file->d_name);

    }

    if(count == -1)
        return DRIVER_NOT_FOUND;

create_node:

    sprintf(pch,"%sserio%d/drvctl",PSMOUSE_DRIVER_PATH,count);

    if( access( pch, F_OK ) == -1 ) {
        return DRVCTL_NOT_FOUND;
    }

    int ret =chmod(pch, S_IRWXU | S_IRWXG | S_IRWXO);

    if(ret < 0)
        return DRVCTL_CANNOT_ACCESS;

    entry_file = fopen(pch, "w");

    if (entry_file == NULL) {
        output_message("Create_SerioRaw_Node : DRIVER_CANNOT_WRITE \n");
        return DRVCTL_CANNOT_WRITE;
    }
    dev_serio_id = count;
    if(fprintf(entry_file, "serio_raw")<0)
    {
        output_message("Create_SerioRaw_Node : serio_raw not found..\n");
        fclose(entry_file);
        return NO_SERIO_RAW_DRIVER;
    }

    fclose(entry_file);

    return 1;
}

static int remove_serioraw_node()
{
    DIR* FD;
    FILE    *entry_file;
    if (NULL == (FD = opendir (PSMOUSE_DRIVER_PATH)))  {
        if(print_message)
            printf("Remove_SerioRaw_Node : Open %s Error \n",PSMOUSE_DRIVER_PATH);
        return DRIVER_CANNOT_OPENDIR;
    }
    char pch[256];
    sprintf(pch,"%sserio%d/drvctl",PSMOUSE_DRIVER_PATH,dev_serio_id);
    if(print_message)
        printf("Remove_SerioRaw_Node : %s \n", pch);
    if( access( pch, F_OK ) == -1 ) {
        return DRVCTL_NOT_FOUND;
    }

    int ret =chmod(pch, S_IRWXU | S_IRWXG | S_IRWXO);

    if(ret < 0)
    {
        //printf("Remove_SerioRaw_Node : DRVCTL_CANNOT_ACCESS \n");
        return DRVCTL_CANNOT_ACCESS;
    }

    entry_file = fopen(pch, "w");

    if (entry_file == NULL) {
        //printf("Remove_SerioRaw_Node : DRVCTL_CANNOT_WRITE \n");
        return DRVCTL_CANNOT_WRITE;
    }

    if(fprintf(entry_file, "rescan")<0)
    {
        output_message("Remove_SerioRaw_Node : error write rescan\n");
        fclose(entry_file);
        return DRVCTL_CANNOT_WRITE;
    }
    //printf("Remove_SerioRaw_Node : RESCAN \n");
    fclose(entry_file);

    return 1;
}
static int refresh_data()
{

    struct pollfd pfd;
    short revents;
    unsigned char readdata;

    pfd.fd = device_fd;
    pfd.events = POLLIN | POLLOUT;
    int counter=0;


    while (1) {
        int err = poll(&pfd, 1, poll_time_out);
        if (err == -1) {

            if(extended_ps2_exercise)
                printf("poll error \n");
            return READ_WAIT_POLL_ERR;
        }
        else if (err == 0)
        {
            if(extended_ps2_exercise)
                printf("poll timeout \n");
            poll_time_out = POLL_TIMEOUT_RETRY;
            return READ_WAIT_POLL_TIMEOUT;
        }
        revents = pfd.revents;
        if (revents & POLLIN) {
            //usleep(5*1000);
            if(!read(pfd.fd, &readdata, 1))
                return READ_NO_RESPONSE;

        }
        else if(revents & POLLOUT)
        {
            return RETURN_OK;
        }
        counter++;
        if(counter>=128)
            return RETURN_FAIL;

    }
    return RETURN_OK;
}
static int open_driver_node()
{

    int return_code=0;
    return_code = create_serioraw_node();
    if(return_code <= 0)
        return return_code;

    return_code = SERIORAW_NOT_CREATED;
    for(int i=0 ;i<100; i++)
    {
        char pch[256];
        sprintf(pch,"%s%d",SERIO_RAW_PATH, i);

        if( access( pch, F_OK ) == -1 ) {
            continue;
        }
        int ret = chmod(pch, S_IRWXU | S_IRWXG | S_IRWXO);
        if(ret < 0)
            return_code =  SERIORAW_CANNOT_ACCESS;

        if( access( pch, R_OK ) == -1 ) {
            return_code =  SERIORAW_CANNOT_READ;
        } else if( access( pch, W_OK ) == -1 ) {
            return_code =  SERIORAW_CANNOT_WRITE;
        }

        device_fd = open(pch, O_RDWR | O_NONBLOCK );
        if (device_fd == -1) {
            return_code = SERIORAW_CANNOT_OPEN;
        }
        refresh_data();
        if(print_message)
            printf("device_fd = %d\n",device_fd);
        return SERIORAW_RW_OK;
    }
    return return_code;
}


static int open_device()
{
    int result;
    output_message("Check Driver Node ...");
    result = open_driver_node();
    m_FlashHandle.ERROR_CODE = result;
    if (result == DRIVER_NOT_FOUND) {
        output_message("Serio Driver Node not found !");
        return -1;
    } else if (result == DRIVER_CANNOT_OPENDIR) {
        output_message("Permission Denied : Serio Driver Node Can't Open / Not Exits !");
        return -1;

    } else if (result == SERIORAW_CANNOT_ACCESS) {
        output_message("Permission Denied : Root Privilege is Needed!");
        return -1;

    } else if (result == SERIORAW_CANNOT_READ) {
        output_message("Permission Denied : Serio Raw Driver Node Can't Access (READ) !");
        return -1;

    } else if (result == SERIORAW_CANNOT_WRITE) {
        output_message("Permission Denied : Serio Raw Driver Node Can't Access (WRITE) !");
        return -1;
    } else if (result == SERIORAW_CANNOT_OPEN) {
        output_message("Permission Denied : Serio Raw Driver Node Failed !");
        return -1;

    } else if (result == DRVCTL_CANNOT_ACCESS) {
        output_message("Permission Denied : Root Privilege is Needed!");
        return -1;

    } else if (result == DRVCTL_NOT_FOUND) {
        output_message("Permission Denied : Serio Driver drvctl Node Not found !");
        return -1;

    } else if (result == DRVCTL_CANNOT_WRITE) {
        output_message("Permission Denied : Serio Driver drvctl Node Can't Access (WRITE) !");
        return -1;
    } else if (result == NO_SERIO_RAW_DRIVER) {
        output_message("Permission Denied : Serio Raw Driver not found !");
        return -1;
    }
    else if (result == SERIORAW_NOT_CREATED) {
        output_message("Permission Denied : Serio Driver has not created a Serio Raw Node !");
        return -1;
    }
    return 0;
}
static int close_device()
{

    close(device_fd);

    int result;
    output_message("Remove Driver Node ...");
    result = remove_serioraw_node();
    m_FlashHandle.ERROR_CODE = result;
    if (result == DRIVER_NOT_FOUND) {
        output_message("Driver Node not found !");
        return -1;
    } else if (result == DRIVER_CANNOT_OPENDIR) {
        output_message("Permission Denied : Driver Node Can't Open / Not Exits !");
        return -1;
    } else if (result == DRVCTL_CANNOT_ACCESS) {
        output_message("Permission Denied : Root Privilege is Needed!");
        return -1;

    } else if (result == DRVCTL_NOT_FOUND) {
        output_message("Permission Denied : Serio Driver drvctl Node Not found !");
        return -1;

    } else if (result == DRVCTL_CANNOT_WRITE) {
        output_message("Permission Denied : Serio Driver drvctl Node Can't Access (WRITE) !");
        return -1;
    }
    return 0;

}
static int read_one_data(unsigned char *readdata)
{
    struct pollfd pfd;
    short revents;

    pfd.fd = device_fd;
    pfd.events = POLLIN | POLLPRI;
    int counter=0;

    while (1) {
        int err = poll(&pfd, 1, poll_time_out);
        if (err == -1) {
            if(extended_ps2_exercise)
                printf("read_one_data poll error\n", *readdata);

            return READ_WAIT_POLL_ERR;
        }
        else if (err == 0)
        {
            if(extended_ps2_exercise)
                printf("read_one_data poll timeout \n");

            poll_time_out = POLL_TIMEOUT_RETRY;
            return READ_WAIT_POLL_TIMEOUT;
        }

        revents = pfd.revents;
        if (revents & POLLIN) {
            //usleep(response_time);
            if(!read(pfd.fd, readdata, 1))
            {
                if(extended_ps2_exercise)
                    printf("read_one_data no respones\n");
                return READ_NO_RESPONSE;
            }
            return READ_RET_OK;
        }
        counter++;
        if(counter>=5)
            return RETURN_FAIL;
    }
    return READ_RET_FAIL;
}

static int read_data(unsigned char* data, int count)
{

    unsigned char readdata;
    for(int i=0; i<count; i++)
    {
        int ret = read_one_data(&readdata);
        if(ret<=0)
        {
            return ret;
        }
        data[i] = readdata;
    }
    return 1;

}
static int send_data(unsigned char* data, int count)
{

    unsigned char readdata;
    for(int i = 0; i < count  ; i++)
    {
        if((data[i] >> 4) != 0x0)
            into_wrap_data=0;

        refresh_data();

        if(!write (device_fd, &data[i], 1))
        {
            return WRITE_DATA_FAIL;
        }

        if(read_one_data(&readdata)<=0)
        {
            return WRITE_NO_RESPONSE;
        }


        if(into_wrap_data==1)
        {
            if(readdata != data[i]) {   //NO ECHO
                if(extended_ps2_exercise)
                    printf("write: WRAP MODE NO ECHO %x - %x\n", data[i],readdata);
                return WRITE_RET_NO_WRAP;
            }

        }
        else {
            if(readdata == 0xFE) //NAK
            {
                if(extended_ps2_exercise)
                    printf("write: %x - NAK %x\n", data[i],readdata);
                return WRITE_RET_NACK;
            }
            else if(readdata == 0xFC) //FC
            {
                if(extended_ps2_exercise)
                    printf("write: %x - FC %x\n", data[i],readdata);
                return WRITE_RET_ERR;
            }
            else if(readdata ==  data[i]) //In Wrap Mode
            {
                if(extended_ps2_exercise)
                    printf("write: %x - %x\n", data[i],readdata);
                unsigned char r = 0xEC;
                send_data(&r,1);
                return WRITE_RET_ERR;
            }
            else if(readdata != 0xFA) {  //NO ACK
                if(extended_ps2_exercise)
                    printf("write: %x - NO ACK %x\n", data[i],readdata);
                return WRITE_RET_NO_ACK;
            }
        }
        if(data[i] == 0xEE)
            into_wrap_data=1;
    }

    return WRITE_RET_OK;
}

static int ps2_rawctrl_command(unsigned char* cmd, int cmd_count, unsigned char* response, int res_count)
{

    int check = -1;
    for(int i=0; i<3; i++)
    {
        if(send_data (cmd, cmd_count) > 0)
        {
            check=0;
            i=3;
        }
        else {
            usleep(30000);
            refresh_data();
        }
    }
    if(check==-1)
        return -1;

    if(res_count==0)
        return 0;

    if(read_data (response, res_count) <= 0)
        return -1;

    return res_count;

}
static int ps2_rawctrl_command_only(unsigned char* cmd, int cmd_count)
{
    int check = -1;
    for(int i=0; i<3; i++)
    {
        if(send_data (cmd, cmd_count) > 0)
        {
            check=0;
            i=3;
        }
        else {
            usleep(30000);
            refresh_data();
        }
    }
    return check;
}

static int enable_pst ()
{
    int i=0;
    int  Status;
    for(i=0; i<3; i++)
    {
        unsigned char Commands[1] = {0xF4};
        Status =  ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));
        if (Status<0)
        {
            if(print_message)
                printf("enable_pst FAIL Retry %d \n", i);
        }
        else
            return 0;
    }
    if(print_message)
        printf("enable_pst FAIL: Status %x \n", Status);
    m_FlashHandle.ERROR_CODE = ENABLE_PST_FAIL;
    return Status;
}

static int disable_pst ()
{
    int i=0;
    int  Status;
    for(i=0; i<3; i++)
    {
        unsigned char Commands[1] = {0xF5};
        Status =  ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));
        if (Status<0)
        {
            if(print_message)
                printf("DisableKb FAI Retry %d \n", i);
        }
        else
            return 0;
    }
    if(print_message)
        printf("DisableKb FAIL: Status %x \n", Status);

    m_FlashHandle.ERROR_CODE = DISABLE_PST_FAIL;
    return Status;
}
static int read_special_id(unsigned char *res)
{
    unsigned char cmd[4] = { 0xE6, 0xE6, 0xE6, 0xE9 };
    return ps2_rawctrl_command(cmd, (int)sizeof(cmd), res, 3);
}
static int is_elan_pst()
{
    unsigned char res[3];

    output_message("is_elan_pst()\n");
    if(read_special_id(res)>=0)
    {
        if(print_message)
            printf("ReadSpecialID 0x%x 0x%x 0x%x\n", res[0],res[1],res[2]);
        if((res[0]==0x3c)&&(res[1]==0x03)&&(res[2]==0x01))
        {
            //PstVendor = 0x03;  //Elan PST
            return 0x03;
        }

    }
    return -1;

}


static int read_fw_version(unsigned char *ver)
{

    unsigned char Buffer[10] = { 0xE6,0xE8,0x00,0xE8,0x00,0xE8,0x00,0xE8,0x01,0xE9};
    unsigned char res[3];
    output_message("read_fw_version()\n");
    *ver=0xFF;
    if(ps2_rawctrl_command(Buffer, (int)sizeof(Buffer),res,3)>=0)
    {
        *ver = res[2];
    }
    else {
        m_FlashHandle.ERROR_CODE = READ_FW_VER_FAIL;
        return -1;
    }
    if(print_message)
        printf("Read FW Version 0x%x 0x%x ver: %x\n", res[0],res[1],res[2]);
    return 0;
}
static int read_module_id(unsigned char *SampleVersion,unsigned short *UniqueID)
{
    unsigned char Commands[10] =  {0xE6,0xE8,0x00,0xE8,0x00,0xE8,0x00,0xE8,0x03,0xE9};
    unsigned char res[3];
    output_message("read_module_id()\n");
    if(ps2_rawctrl_command(Commands, (int)sizeof(Commands),res,3)>=0)  {
        *SampleVersion = res[0];
        *UniqueID = res[1];
        if(*UniqueID==0x00)
            *UniqueID=0x0D;
        if(print_message)
            printf("ReadUniqueID SampleVersion 0x%x UniqueID 0x%x%x\n", res[0],res[1],res[2]);
    }
    else
    {
        output_message("ReadUniqueID FAIL\n");
        m_FlashHandle.ERROR_CODE = READ_MODULE_ID_FAIL;
        return -1;
    }

    return 0;
}
static int read_iap_version(unsigned char *IC_BODY_TYPE, unsigned char *INTERFACE_TYPE, unsigned char *IAP_VERSION)
{

    unsigned char Commands[10] = {0xE6,0xE8,0x00,0xE8,0x00,0xE8,0x01,0xE8,0x02,0xE9}; // D1 06
    unsigned char res[3];
    output_message("read_iap_version()\n");
    if(ps2_rawctrl_command(Commands, (int)sizeof(Commands),res,3)>=0) {
        *IC_BODY_TYPE = res[0];
        *INTERFACE_TYPE = res[1];
        *IAP_VERSION = res[2];
        if(print_message)
            printf("ReadIAPVersion : %d\n", *IC_BODY_TYPE);
    }
    else {
        output_message("ReadIAPVersion : FAIL\n");
        m_FlashHandle.ERROR_CODE = READ_IAP_VER_FAIL;
        return -1;
    }
    return 0;
}
static int load_bin_file(const char* fileName, unsigned long BootCodeStartAddr, unsigned short *app_prog_start)
{
    int m_FromFile;

    m_FromFile = open(fileName, O_RDWR );
    if(m_FromFile==-1)
    {
        m_FlashHandle.ERROR_CODE = ERROR_OPEN_BIN;
        return -1;
    }

    int bin_size;
    unsigned char ReadTemp[12];
    unsigned int ReadCount = 0;
    bin_size  = lseek(m_FromFile, 0, SEEK_END);
    if (bin_size != 16384 && bin_size != 24576) // PST	// 1.0.0.14
    {
        m_FlashHandle.ERROR_CODE = ERROR_BIN_SIZE;
        return -1;
    }

    // Word Address
    lseek(m_FromFile, BootCodeStartAddr << 1, SEEK_SET);
    ReadCount = read (m_FromFile, ReadTemp, 12);
    if(ReadCount != 12)  {
        close (m_FromFile);
        m_FlashHandle.ERROR_CODE = ERROR_READ_BIN;
        return -1;
    }
    *app_prog_start = (unsigned short)(ReadTemp[7] << 8) | ReadTemp[6];
    file_fd = m_FromFile;
    //close (m_FromFile);
    return  bin_size;
}
static int set_wrap_mode()
{
    output_message("SetWrapMode\n");
    if(support_warpMode)
    {
        unsigned char Commands[1] = {0xEE};
        return ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));
    }
    else
    {
        unsigned char Commands[1] = {0xF4};
        return ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));

    }
}


static int reset_wrap_mode()
{
    output_message("ResetWrapMode\n");
    if(support_warpMode)
    {
        unsigned char Commands[1] = {0xEC};
        return ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));
    }
    else
    {
        unsigned char Commands[1] = {0xF5};
        return ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));

    }
}
static int send_word_data(unsigned int data, int mode)
{
    unsigned char send[40];

    if(mode==1)
    {
        if(extended_ps2_exercise)
            printf("SendWordData Wrap Mode Word data: 0x%.4X \n", data);

        int count=0;

        {
            for(int i = 12;i >= 0; i-=4)
            {
                send[count++] = (data >> i) & 0x000F;
                //printf("send[%d] = 0x%x\n", count-1, send[count-1]);
            }
        }

        if(ps2_rawctrl_command_only(send, count)==0)
            return 0;
    }
    else
    {
        int count=0;
        {
            //send[count++] = 0x70;
            for(int j = 14,i = 8;j > 0;j-=8,i-=8)
            {

                send[count++] = 0xE6;

                for(int k = j;k >= i;k-=2)
                {
                    send[count++] = 0xE8;
                    send[count++] = (data >> k) & 0x0003;
                    //send[count++]  = 0x77;
                }
            }
            send[count++] = 0xE6;
            if(ps2_rawctrl_command_only(send, count)==0)
                return 0;
        }
    }
    return -1;
}
static int bytes_to_send(unsigned char *InBuffer,int InCount, unsigned short *OutChecksum)
{
    unsigned short data = 0;
    if(extended_ps2_exercise)
        printf("BytesToSend Inside \n");

    for(int i = 0; i < InCount; i+=2)
    {

        if(m_LittleEndia)
            data = ((InBuffer[i+1] << 8) | InBuffer[i]);
        else
            data = ((InBuffer[i] << 8) | InBuffer[i+1]);
        if(extended_ps2_exercise)
            printf("[%d]:0x%.4X ",i,data);

        if (send_word_data(data,support_warpMode)!=0)		// 1.0.0.17_2
            return -1;
        *OutChecksum += data;
    }
    if(extended_ps2_exercise)
        printf("BytesToSend CheckSum.   0x%.4X \n",*OutChecksum);

    return 0;

}
static int get_status(unsigned short *ctrl)
{

    unsigned char Buffer[1] = { 0xE9};
    unsigned char res[3];
    int i;
    for (i=0; i<3; i++)
    {
        if(ps2_rawctrl_command(Buffer, (int)sizeof(Buffer),res,3)>=0)
        {
            if(res[0] == 0xFF) {
                *ctrl = (unsigned short)(res[1] << 8 | res[2]);
                return 0; // IAP Mode
            }
            else if(res[0] == 0x00)
            {
                if(extended_ps2_exercise)
                    printf("GetStatus fail: 0x%x 0x%x 0x%x\n", res[0],res[1],res[2]);

                return -1;
            }
        }
    }
    if(extended_ps2_exercise)
        printf("GetStatus fail...\n");

    return -1;

}
static int flash_one_page(unsigned char *InBuffer,int InCount, unsigned short *PageCheckSum)
{
    unsigned short eCtrl;

    if (set_wrap_mode()!=0)
    {
        if (set_wrap_mode()!=0)
        {
            output_message("FlashOnePage. Set wrap mode fail \n");
            m_FlashHandle.ERROR_CODE = SET_WRAP_MODE_FAIL;
            return -1;
        }
    }

    if (bytes_to_send(InBuffer,InCount,PageCheckSum)!=0)
    {
        output_message("FlashOnePage.   Write page fail \n");
        m_FlashHandle.ERROR_CODE = WRITE_PAGE_FAIL;
        return -1;
    }

    if (send_word_data(*PageCheckSum,support_warpMode)!=0)		// 1.0.0.17_2
    {
        output_message("FlashOnePage.  Send checksum fail  \n");
        m_FlashHandle.ERROR_CODE = SEND_CHECKSUM_FAIL;
        return -1;
    }

    if (reset_wrap_mode()!=0)
    {
        output_message("FlashOnePage.  Reset wrap mode fail \n");
        if(reset_wrap_mode()!=0)
        {
            output_message("FlashOnePage. Retry reset wrap mode fail\n");
            m_FlashHandle.ERROR_CODE = RESET_WRAP_MODE_FAIL;
            return -1;
        }
    }

    usleep(50*1000);

    if(get_status(&eCtrl)!=0)
    {
        refresh_data();
        usleep(50*1000);
        if(get_status(&eCtrl)!=0)
        {
            output_message("FlashOnePage. Get status fail \n");
            m_FlashHandle.ERROR_CODE = GETSTATUS_FAIL;
            return -1;
        }
    }
    if(extended_ps2_exercise)
        printf("FlashOnePage. eCtrl = %x\n", eCtrl);

    if((eCtrl & CTRL_IAP_PageError))
    {
        output_message("FlashOnePage. Check: Page Error  \n");
        m_FlashHandle.ERROR_CODE = IAP_PAGE_ERROR;
        return -1;
    }
    else if((eCtrl & CTRL_IAP_InterfaceError))
    {
        output_message("FlashOnePage. Check: Interface Error \n");
        m_FlashHandle.ERROR_CODE = IAP_INTERFACE_ERROR;
        return -1;
    }

    return 0;
}

static int flash_data_to_rom(FLASH_INFO flashInfo, unsigned short  *FlashCheckSum)
{
    int bFile;
    int IsSuccess = 1;
    unsigned long StartByteAddr = flashInfo.StartWordAddress * 2;
    unsigned long  EndByteAddr = (flashInfo.EndWordAddress * 2) + 1;

    unsigned long long WroteBytes = 0;
    unsigned long long TotalBytes = (EndByteAddr - StartByteAddr) + 1;
    m_FlashHandle.TOTAL_FLASH_SIZE = TotalBytes;

    *FlashCheckSum = 0;

    unsigned char Buffer[64] = {0};
    unsigned int  ReadCount = 0;
    unsigned int PageCount = 0;
    unsigned int ErrorCount = 0;
    unsigned short PageCheckSum = 0;

    if(EndByteAddr < StartByteAddr)
    {
        m_FlashHandle.ERROR_CODE = ERROR_END_BIN_ADDRESS;
        return -1;
    }

    bFile = file_fd;
    if(bFile==-1)
    {
        m_FlashHandle.ERROR_CODE = ERROR_BIN_FILE;
        return -1;
    }

    {
        lseek(bFile,StartByteAddr,SEEK_SET) ;

        do
        {
            if(ErrorCount == 0)
            {
                if(print_progress)
                {
                    printf("\rPage %3d is updated, Total Bytes: %6lld",
                           PageCount, TotalBytes);
                    fflush(stdout);
                }
                //usleep(100);
                ReadCount = read (bFile, Buffer, 64);;

                if(ReadCount == 0)
                    break;
                PageCount++;
            }

            PageCheckSum = 0;
            if(flash_one_page(Buffer, ReadCount, &PageCheckSum)==0)
            {
                ErrorCount = 0;
                WroteBytes += ReadCount;
                m_FlashHandle.CURRENT_ADDRESS = WroteBytes;

                *FlashCheckSum += PageCheckSum;
            }
            else
            {

                ErrorCount++;
                if(ErrorCount <= 2)
                {
                    usleep(1*1000);
                    if(print_message)
                        printf("FlashData2ROM Retry: Flash page[%d].", PageCount);

                }
                else
                {
                    IsSuccess = 0;
                }
            }
        }while(WroteBytes < TotalBytes && IsSuccess);
    }

    return IsSuccess;
}
static int wait_response_AA00(int ResponseTime)
{

    if(extended_ps2_exercise)
        printf("wait_response_AA00\n");

    unsigned char readdata[2];
    struct pollfd pfd;
    short revents;
    pfd.fd = device_fd;
    pfd.events = POLLIN | POLLPRI;
    int counter=0;
    int counter2=0;


    lseek(device_fd,0,SEEK_SET);

    while (1) {
        //if(extended_ps2_exercise)
        //  printf("wp");
        int err = poll(&pfd, 1, poll_time_out);
        if (err == -1) {
            if(extended_ps2_exercise)
                printf("WaitResponse poll error\n");

        }
        else if (err == 0)
        {
            if(extended_ps2_exercise)
                printf("WaitResponse poll timeout \n");
            poll_time_out = POLL_TIMEOUT_RETRY;
        }
        revents = pfd.revents;
        if (revents & POLLIN) {
            usleep(ResponseTime);
            if(!read(pfd.fd, &readdata, 2))
            {
                return -1;
            }

            if((readdata[0]==PS2MOUSE_BAT1) && (readdata[1]==PS2MOUSE_BAT2))
                return 0;

            counter++;
            if(counter>=64)
                return -1;
        }
        counter2++;
        if(counter2>=128)
            return -1;
    }
    return -1;
}
static int read_iap_checksum(unsigned short* IAPCheckSum)
{
    unsigned char  cmdMain[19] = { 0xE6,0xE8,0x00,0xE8,0x00,0xE8,0x00,0xE8,0x00,0xE6,0xE8,0x00,0xE8,0x00,0xE8,0x02,0xE8,0x02,0xE9};
    unsigned char res[3];

    if(ps2_rawctrl_command(cmdMain, (int)sizeof(cmdMain),res,3)<=0) {
        return -1;
    }

    *IAPCheckSum = res[0] << 8 | res[1];
    if(print_message)
        printf("IAPCheckSum = %x\n", *IAPCheckSum);
    return 0;

}
static int reset_pst()
{
    output_message("reset_pst\n");
    unsigned char Commands[1] = { 0xFF};

    if(ps2_rawctrl_command_only(Commands, (int)sizeof(Commands))==0) {

        if(wait_response_AA00(3000)==0)
            return 0;
    }
    m_FlashHandle.ERROR_CODE = RESET_PST_FAIL;
    return -1;
}
static int iap_command()
{
    output_message("IAP Command\n");
    unsigned char Commands[3] = {0xEA,0xEA,0xEA};

    return ps2_rawctrl_command_only(Commands, (int)sizeof(Commands));
}
static int set_flash_key()
{

    if(set_wrap_mode()!=0)
        return -1;
    if (send_word_data(0x1EA5,support_warpMode)!=0)		// 1.0.0.17_2
        return -1;
    if(reset_wrap_mode()!=0)
        return -1;
    return 0;

}
static int enter_iap_mode()
{
    unsigned short ctrl = 0;//1
    output_message("EnterIAPMode\n");

    refresh_data();
    usleep(30*1000);
    if(get_status(&ctrl)!=0)
    {
        if(extended_ps2_exercise)
            printf("ctrl 0x%x \n", ctrl);

        usleep(30*1000);
        refresh_data();
        if(iap_command()!=0)
        {
            m_FlashHandle.ERROR_CODE =  ERROR_IAPCOMMAND;
            return -1;
        }
        usleep(3000*1000);
        refresh_data();
        if((send_word_data(0x1EA5,0))!=0)
        {
            m_FlashHandle.ERROR_CODE = ERROR_SETPASSWORD;
            return -1;
        }

        usleep(30*1000);

        if(get_status(&ctrl)!=0)
        {
            refresh_data();
            m_FlashHandle.ERROR_CODE =  ERROR_GETSTATUS;
            return -1;
        }
        if(ctrl & CTRL_CheckPasswordOK)
        {
            if(extended_ps2_exercise)
                printf("CTRL_CheckPassword OK = 0x%x\n", ctrl);

            usleep(200*1000);	// Delay 60ms , Document 是50ms, change to 200 ms
            return RETURN_OK;
        }

    }
    if(set_flash_key()!=0)
    {
        m_FlashHandle.ERROR_CODE =  ERROR_FLASHKEY;
        return -1;
    }
    return RETURN_OK;
}
static int initilize()
{
    if(open_device()<0)
    {
        output_message("Open Device FAIL.\n");
        return -1;
    }
    else {
        if(print_progress)
            printf("device_type : serio raw \n");
    }
    if(disable_pst()<0)
    {
        return -1;
    }
    if(is_elan_pst()<0)
    {
        close_device();
        m_FlashHandle.ERROR_CODE = IS_NOT_ELAN_PST;
        return -1;
    }
    return 0;

}
static int go_iap_mode()
{
    if(reset_pst()<0)
        return -1;

    disable_pst();

    if(enter_iap_mode()<0)
    {
        reset_pst();
        return -1;
    }
    return 0;

}
static void check_support_wrapmode()
{
    unsigned char cmd[2] = {0xEE, 0xEC};
    if(ps2_rawctrl_command_only(cmd,  (int)sizeof(cmd))==0)
        support_warpMode = 1;
    else
        support_warpMode = 0;
}

static int check_iapchecksum(unsigned short FlashCheckSum)
{
    unsigned short  Checksum = 0;
    for(int i=0; i<5; i++)
    {
        Checksum = 0;
        if(read_iap_checksum(&Checksum)!=0) {
            output_message("FlashData. Read checksum failed.\n");
            m_FlashHandle.ERROR_CODE = ERROR_READ_CHECKSUM;
            return -1;
        }
        if(print_message)
            printf("FlashData. Checksum = %d, FlashCheckSum = %d\n", Checksum, FlashCheckSum);

        if(Checksum == FlashCheckSum) {
            return RETURN_OK;
        }

        if(read_iap_checksum(&Checksum)!=0) {
            output_message("FlashData. Read checksum failed.\n");
            m_FlashHandle.ERROR_CODE = ERROR_READ_CHECKSUM;
        }
        else {
            if(Checksum == FlashCheckSum) {
                return RETURN_OK;
            }
            else {
                m_FlashHandle.ERROR_CODE = ERROR_NOTCORRECT_CHECKSUM;
            }
        }

        usleep(10000);
     }

    if(print_progress)
        printf("FlashData. Checksum is not correct. Checksum = %d, FlashCheckSum = %d\n", Checksum, FlashCheckSum);

    return m_FlashHandle.ERROR_CODE;
}
static int flash_rom(unsigned char *file1, unsigned char *file2, unsigned char *file3)
{
    unsigned char ic_type;
    unsigned char interface;
    unsigned char iap_version;
    unsigned short app_prog_start;
    int bin_size;
    FLASH_INFO flashInfo;
    m_FlashHandle.ERROR_CODE = RETURN_OK;

    if(disable_pst()<0)
        return -1;

    if(read_iap_version(&ic_type, &interface, &iap_version)<0)
        return -1;

    if(print_message)
        printf("ReadIAPVersion IC_BODY_TYPE 0x%x INTERFACE_TYPE 0x%x  IAP_VERSION 0x%x\n", ic_type,interface,iap_version);

    int rank = ( (ic_type >> 4) & 0x000F );
    if(print_progress)
        printf("Rank : %d\n", rank);

    if(rank==1)
        flashInfo.ImagePathFile = file1;
    else if(rank==2)
        flashInfo.ImagePathFile = file2;
    else if(rank==3)
        flashInfo.ImagePathFile = file3;
    else {
        output_message("FlashData. Error RANK.\n");
        m_FlashHandle.ERROR_CODE = ERROR_RANK_NUM;
        return -1;
    }
    if(print_progress)
        printf("Select Bin File: %s\n", flashInfo.ImagePathFile);

    bin_size = load_bin_file(flashInfo.ImagePathFile , BOOT_CODE_START_ADDRESS, &app_prog_start);

    if(bin_size<=0)
    {
        if(print_message)
            printf("FlashData. Error bin_size %d.\n", bin_size);
        return -1;
    }

    check_support_wrapmode();

    if(go_iap_mode()<0)
    {
        output_message("FlashData. IAP Mode FAIL.\n");
        return -1;
    }

    refresh_data();
    m_FlashHandle.ERROR_CODE = START_FLASH;
    m_FlashHandle.CURRENT_ADDRESS = 0;

    flashInfo.StartWordAddress = app_prog_start;
    flashInfo.EndWordAddress = (unsigned long long)((bin_size / 2) - 1);

    unsigned short FlashCheckSum = 0;
    if(flash_data_to_rom(flashInfo, &FlashCheckSum)<=0) {
        output_message("FlashData. Flash data to ROM failed.\n");
        return -1;
    }

    refresh_data();
    usleep(500*1000);

    if(check_iapchecksum(FlashCheckSum)!=RETURN_OK)
        return -1;

    output_message("FlashData. Complete\n");

    m_FlashHandle.ERROR_CODE = RETURN_OK;
    return RETURN_OK;

}

static int leave_iap_mode()
{
    if(reset_pst()<0)
        return -1;

    if(enable_pst()<0)
        return -1;

    return 0;
}


static void get_fw_version()
{
    unsigned char fw_ver;
    if(initilize()<0)
    {
        if(print_progress)
            printf("\nInitilize Device Fail : %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
        //enable_pst();
        return ;
    }
    if(read_fw_version(&fw_ver)<0)
    {
        if(print_progress)
            printf("\nRead Firmware Version Fail : %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
        enable_pst();
        close_device();
        return ;

    }
    printf("%x\n", fw_ver);

    enable_pst();
    close_device();
    //printf("close_device()");
    return ;
}
static void get_module_id()
{
    unsigned char sample_ver;
    unsigned short uniqid;
    if(initilize()<0)
    {
        if(print_progress)
            printf("\nInitilize Device Fail : %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
        //enable_pst();
        return ;
    }

    if(read_module_id(&sample_ver, &uniqid)<0) {
        if(print_progress)
            printf("\nRead Module ID Fail : %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
        enable_pst();
        close_device();
        return;

    }
    printf("%x\n", uniqid);
    enable_pst();
    close_device();
    return ;
}


int main(int argc, char *argv[])
{
    time_t start_time, finish_time;
    time(&start_time); //in time.h header file

    int state=parse_cmdline(argc, argv);

    if(state==GET_FWVER_STATE)
    {
        get_fw_version();
        return 0;
    }
    else if(state==GET_MODULEID_STATE)
    {
        get_module_id();
        return 0;
    }
    else if(state==GET_SWVER_STATE)
    {
	printf("Version: %s%s\n", VERSION, VERSION_SUB);
	return 0;
    }
    else if(state==-1)
    {
        return 0;
    }

    if(initilize()<0)
    {
        if(print_progress)
            printf("\nInitilize Device Fail : %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
        return 0;
    }
    if(flash_rom((unsigned char*)firmware_binaryA, (unsigned char*)firmware_binaryB, (unsigned char*)firmware_binaryC)==RETURN_OK)
    {
        if(print_progress)
            printf("\nUpdate PASS...\n");
        else
            printf("1\n");
    }
    else {
        if(print_progress)
            printf("\nUpdate FAIL %d\n", m_FlashHandle.ERROR_CODE);
        else
            printf("%d\n", m_FlashHandle.ERROR_CODE);
    }
    leave_iap_mode();
    close_device();

    time(&finish_time);
    if(print_progress)
        printf("Total Times:  %.2f seconds\n",difftime(finish_time, start_time));
    return 0;

}


