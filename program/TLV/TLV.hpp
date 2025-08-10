enum TaskType{
    TASK_TYPE_LOGIN_SECTION1,
    TASK_TYPE_LOGIN_SECTION1_RESP_OK,
    TASK_TYPE_LOGIN_SECTION1_RESP_ERROR,
    TASK_TYPE_LOGIN_SECTION2,
    TASK_TYPE_LOGIN_SECTION2_RESP_OK,
    TASK_TYPE_LOGIN_SECTION2_RESP_ERROR,

}

class TLV{
    private:
    int _type;
    int _length;
    char _data[1024];

    public:

};