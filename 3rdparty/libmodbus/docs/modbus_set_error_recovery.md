# modbus_set_error_recovery

## Name

modbus_set_error_recovery - set the error recovery mode

## Synopsis

```c
int modbus_set_error_recovery(modbus_t *ctx, modbus_error_recovery_mode error_recovery);
```

## Description

The *modbus_set_error_recovery()* function shall set the error recovery mode to
apply when the connection fails or the byte received is not expected. The
argument `error_recovery` may be bitwise-or'ed with zero or more of the
following constants.

By default there is no error recovery (`MODBUS_ERROR_RECOVERY_NONE`) so the
application is responsible for controlling the error values returned by
libmodbus functions and for handling them if necessary.

When `MODBUS_ERROR_RECOVERY_LINK` is set, the library will attempt an
reconnection after a delay defined by response timeout of the libmodbus context.
This mode will try an infinite close/connect loop until success on send call and
will just try one time to re-establish the connection on select/read calls (if the
connection was down, the values to read are certainly not available any more after
reconnection, except for slave/server). This mode will also run flush requests
after a delay based on the current response timeout in some situations (eg.
timeout of select call). The reconnection attempt can hang for several seconds
if the network to the remote target unit is down.

When `MODBUS_ERROR_RECOVERY_PROTOCOL` is set, a sleep and flush sequence will be
used to clean up the ongoing communication, this can occurs when the message
length is invalid, the TID is wrong or the received function code is not the
expected one. The response timeout delay will be used to sleep.

The modes are mask values and so they are complementary.

It's not recommended to enable error recovery for a Modbus slave/server.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set
errno to one of the values defined below.

## Errors

- *EINVAL*, the value of the argument `error_recovery` is not positive.

## Example

```c
modbus_set_error_recovery(
    ctx,
    MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL
);
```
