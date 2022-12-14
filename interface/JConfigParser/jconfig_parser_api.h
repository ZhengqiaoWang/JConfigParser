#ifndef JCONFIG_PARSER_API_H
#define JCONFIG_PARSER_API_H
#include <iostream>


namespace Joger
{
    namespace ConfigParser
    {
        

        

    /*
     *
     {
        "h1":"abc",
        "h2":{
            "h21":"123",
            "h22":[1,2,3],
            "h23":["1","2","3"]
        },
        "h3":[
            {
                "h311":"123",
                "h312":4321
            },
            {
                "h321":"aaa",
                "h322":234.2
            }
        ]
     } 
     map{
        value,
        map{
            value,
            vec(value),
            vec(value)
        },
        vec(
            map{
                value,
                value
            },
            map{
                value,
                value
            }
        )
     }
     * 
     */
    }
}

#endif