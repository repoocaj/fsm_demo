/**
 * @file    error_msg.h
 *
 * Defines some macros for handling logging of error messages.
 */
#ifndef __X_ERROR_MSG_H
#define __X_ERROR_MSG_H

#include "sdk_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for checking an error code and logging a warning with a
 *          description if the error code is anything other than NRF_SUCCESS.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 * @param[in]   MSG         Message to log if an error.
 * @param[in]   EXIT        Boolean that if true, indicates we should return.
 */
#define WARNING_MSG(ERROR_CODE, MSG)                                        \
    do                                                                      \
    {                                                                       \
        const ret_code_t err = (ERROR_CODE);                                \
                                                                            \
        if (err != NRF_SUCCESS)                                             \
        {                                                                   \
            NRF_LOG_WARNING("%s: %s (%d 0x%x)", (MSG), nrf_strerror_get(err), err, err);\
        }                                                                   \
    } while (0)

/**@brief   Macro for checking an error code and logging and error with a
 *          description if the error code is anything other than NRF_SUCCESS.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 * @param[in]   MSG         Message to log if an error.
 * @param[in]   EXIT        Boolean that if true, indicates we should return.
 */
#define ERROR_MSG(ERROR_CODE, MSG)                                          \
    do                                                                      \
    {                                                                       \
        const ret_code_t err = (ERROR_CODE);                                \
                                                                            \
        if (err != NRF_SUCCESS)                                             \
        {                                                                   \
            NRF_LOG_ERROR("%s: %s (%d 0x%x)", (MSG), nrf_strerror_get(err), err, err);\
        }                                                                   \
    } while (0)

/**@brief   Macro for checking an error code.  Returns from the calling
 *          function with the error code if it is anything other than
 *          NRF_SUCCESS.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 */
#define ERROR_EXIT_RETURN(ERROR_CODE)                                       \
    do                                                                      \
    {                                                                       \
        const ret_code_t err = (ERROR_CODE);                                \
                                                                            \
        if (err != NRF_SUCCESS)                                             \
        {                                                                   \
            return(err);                                                    \
        }                                                                   \
    } while (0)

/**@brief   Macro for checking an error code.  Returns from the calling
 *          function if it is anything other than NRF_SUCCESS.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 */
#define ERROR_EXIT_VOID(ERROR_CODE)                                         \
    do                                                                      \
    {                                                                       \
        const ret_code_t err = (ERROR_CODE);                                \
                                                                            \
        if (err != NRF_SUCCESS)                                             \
        {                                                                   \
            return;                                                         \
        }                                                                   \
    } while (0)

/**@brief   Macro for checking an error code and logging and error with a
 *          description if the error code is anything other than NRF_SUCCESS.
 *          If an error exists, the macro will continue in the calling
 *          function.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 * @param[in]   MSG         Message to log if an error
 */
#define ERROR_MSG_CONT(ERROR_CODE, MSG)                                     \
    ERROR_MSG(ERROR_CODE, MSG)

/**@brief   Macro for checking an error code and logging and error with a
 *          description if the error code is anything other than NRF_SUCCESS.
 *          If an error exists, the macro will return from the calling
 *          function returning the error code.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 * @param[in]   MSG         Message to log if an error
 */
#define ERROR_MSG_EXIT(ERROR_CODE, MSG)                                     \
    do                                                                      \
    {                                                                       \
        ERROR_MSG(ERROR_CODE, MSG);                                         \
        ERROR_EXIT_RETURN(ERROR_CODE);                                      \
    } while (0)

/**@brief   Macro for checking an error code and logging and error with a
 *          description if the error code is anything other than NRF_SUCCESS.
 *          If an error exists, the macro will return from the calling
 *          function.
 *
 * @param[in]   ERROR_CODE  Error code to test.
 * @param[in]   MSG         Message to log if an error
 */
#define ERROR_MSG_EXIT_VOID(ERROR_CODE, MSG)                                \
    do                                                                      \
    {                                                                       \
        ERROR_MSG(ERROR_CODE, MSG);                                         \
        ERROR_EXIT_VOID(ERROR_CODE);                                        \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif  // __X_ERROR_MSG_H

