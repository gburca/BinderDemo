/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
   vi:ai:tabstop=8:shiftwidth=4:softtabstop=4:expandtab
*/

/*
 * Author: Gabriel Burca <gburca dash binder at ebixio dot com>
 *
 * Sample code for using binders in Android from C++
 *
 * The Demo service provides 3 operations: push(), alert(), add(). See
 * the IDemo class documentation to see what they do.
 *
 * Both the server and client code are included below.
 *
 * To view the log output:
 *      adb logcat -v time binder_demo:* *:S
 *
 * To run, create 2 adb shell sessions. In the first one run "binder" with no
 * arguments to start the service. In the second one run "binder N" where N is
 * an integer, to start a client that connects to the service and calls push(N),
 * alert(), and add(N, 5).
 */

#define LOG_TAG "binder_demo"

/* For relevant code see:
    frameworks/native/{include,libs}/binder/{IInterface,Parcel}.{h,cpp}
    system/core/include/utils/{Errors,RefBase}.h
 */

#include <stdlib.h>

#include <utils/RefBase.h>
#include <utils/Log.h>
#include <binder/TextOutput.h>

#include <binder/IInterface.h>
#include <binder/IBinder.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <binder/IPCThreadState.h>

using namespace android;


#define INFO(...) \
    do { \
        printf(__VA_ARGS__); \
        printf("\n"); \
        ALOGD(__VA_ARGS__); \
    } while(0)

void assert_fail(const char *file, int line, const char *func, const char *expr) {
    INFO("assertion failed at file %s, line %d, function %s:",
            file, line, func);
    INFO("%s", expr);
    abort();
}

#define ASSERT(e) \
    do { \
        if (!(e)) \
            assert_fail(__FILE__, __LINE__, __func__, #e); \
    } while(0)


// Where to print the parcel contents: aout, alog, aerr. alog doesn't seem to work.
#define PLOG aout



// Interface (our AIDL) - Shared by server and client
class IDemo : public IInterface {
    public:
        enum {
            ALERT = IBinder::FIRST_CALL_TRANSACTION,
            PUSH,
            ADD
        };
        // Sends a user-provided value to the service
        virtual void        push(int32_t data)          = 0;
        // Sends a fixed alert string to the service
        virtual void        alert()                     = 0;
        // Requests the service to perform an addition and return the result
        virtual int32_t     add(int32_t v1, int32_t v2) = 0;

        DECLARE_META_INTERFACE(Demo);  // Expands to 5 lines below:
        //static const android::String16 descriptor;
        //static android::sp<IDemo> asInterface(const android::sp<android::IBinder>& obj);
        //virtual const android::String16& getInterfaceDescriptor() const;
        //IDemo();
        //virtual ~IDemo();
};

// Client
class BpDemo : public BpInterface<IDemo> {
    public:
        BpDemo(const sp<IBinder>& impl) : BpInterface<IDemo>(impl) {
            ALOGD("BpDemo::BpDemo()");
        }

        virtual void push(int32_t push_data) {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeInt32(push_data);

            aout << "BpDemo::push parcel to be sent:\n";
            data.print(PLOG); endl(PLOG);

            remote()->transact(PUSH, data, &reply);

            aout << "BpDemo::push parcel reply:\n";
            reply.print(PLOG); endl(PLOG);

            ALOGD("BpDemo::push(%i)", push_data);
        }

        virtual void alert() {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeString16(String16("The alert string"));
            remote()->transact(ALERT, data, &reply, IBinder::FLAG_ONEWAY);    // asynchronous call
            ALOGD("BpDemo::alert()");
        }

        virtual int32_t add(int32_t v1, int32_t v2) {
            Parcel data, reply;
            data.writeInterfaceToken(IDemo::getInterfaceDescriptor());
            data.writeInt32(v1);
            data.writeInt32(v2);
            aout << "BpDemo::add parcel to be sent:\n";
            data.print(PLOG); endl(PLOG);
            remote()->transact(ADD, data, &reply);
            ALOGD("BpDemo::add transact reply");
            reply.print(PLOG); endl(PLOG);

            int32_t res;
            status_t status = reply.readInt32(&res);
            ALOGD("BpDemo::add(%i, %i) = %i (status: %i)", v1, v2, res, status);
            return res;
        }
};

    //IMPLEMENT_META_INTERFACE(Demo, "Demo");
    // Macro above expands to code below. Doing it by hand so we can log ctor and destructor calls.
    const android::String16 IDemo::descriptor("Demo");
    const android::String16& IDemo::getInterfaceDescriptor() const {
        return IDemo::descriptor;
    }
    android::sp<IDemo> IDemo::asInterface(const android::sp<android::IBinder>& obj) {
        android::sp<IDemo> intr;
        if (obj != NULL) {
            intr = static_cast<IDemo*>(obj->queryLocalInterface(IDemo::descriptor).get());
            if (intr == NULL) {
                intr = new BpDemo(obj);
            }
        }
        return intr;
    }
    IDemo::IDemo() { ALOGD("IDemo::IDemo()"); }
    IDemo::~IDemo() { ALOGD("IDemo::~IDemo()"); }
    // End of macro expansion

// Server
class BnDemo : public BnInterface<IDemo> {
    virtual status_t onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags = 0);
};

status_t BnDemo::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) {
    ALOGD("BnDemo::onTransact(%i) %i", code, flags);
    data.checkInterface(this);
    data.print(PLOG); endl(PLOG);

    switch(code) {
        case ALERT: {
            alert();    // Ignoring the fixed alert string
            return NO_ERROR;
        } break;
        case PUSH: {
            int32_t inData = data.readInt32();
            ALOGD("BnDemo::onTransact got %i", inData);
            push(inData);
            ASSERT(reply != 0);
            reply->print(PLOG); endl(PLOG);
            return NO_ERROR;
        } break;
        case ADD: {
            int32_t inV1 = data.readInt32();
            int32_t inV2 = data.readInt32();
            int32_t sum = add(inV1, inV2);
            ALOGD("BnDemo::onTransact add(%i, %i) = %i", inV1, inV2, sum);
            ASSERT(reply != 0);
            reply->print(PLOG); endl(PLOG);
            reply->writeInt32(sum);
            return NO_ERROR;
        } break;
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

class Demo : public BnDemo {
    virtual void push(int32_t data) {
        INFO("Demo::push(%i)", data);
    }
    virtual void alert() {
        INFO("Demo::alert()");
    }
    virtual int32_t add(int32_t v1, int32_t v2) {
        INFO("Demo::add(%i, %i)", v1, v2);
        return v1 + v2;
    }
};


// Helper function to get a hold of the "Demo" service.
sp<IDemo> getDemoServ() {
    sp<IServiceManager> sm = defaultServiceManager();
    ASSERT(sm != 0);
    sp<IBinder> binder = sm->getService(String16("Demo"));
    // TODO: If the "Demo" service is not running, getService times out and binder == 0.
    ASSERT(binder != 0);
    sp<IDemo> demo = interface_cast<IDemo>(binder);
    ASSERT(demo != 0);
    return demo;
}


int main(int argc, char **argv) {

    if (argc == 1) {
        ALOGD("We're the service");

        defaultServiceManager()->addService(String16("Demo"), new Demo());
        android::ProcessState::self()->startThreadPool();
        ALOGD("Demo service is now ready");
        IPCThreadState::self()->joinThreadPool();
        ALOGD("Demo service thread joined");
    } else if (argc == 2) {
        INFO("We're the client: %s", argv[1]);

        int v = atoi(argv[1]);

        sp<IDemo> demo = getDemoServ();
        demo->alert();
        demo->push(v);
        const int32_t adder = 5;
        int32_t sum = demo->add(v, adder);
        ALOGD("Addition result: %i + %i = %i", v, adder, sum);
    }

    return 0;
}

/*
    Single-threaded service, single-threaded client.
 */
