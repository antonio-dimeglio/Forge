#pragma once

namespace LLVMLabels {
    constexpr const char* MAIN_LABEL = "main";
    constexpr const char* FUNC_ENTRY = "entry";

    constexpr const char* WHILE_CONDITION = "while.cond";
    constexpr const char* WHILE_BODY = "while.body";
    constexpr const char* WHILE_EXIT = "while.exit";

    constexpr const char* IF_THEN = "if.then";
    constexpr const char* IF_ELSE = "if.else";
    constexpr const char* IF_MERGE = "if.merge";

    constexpr const char* UNIQUE_PTR_RELEASE = "unique_ptr_release";
    constexpr const char* SMART_PTR_RETAIN = "smart_ptr_retain";
    constexpr const char* SHARED_PTR_RETAIN = "shared_ptr_retain";
    constexpr const char* SHARED_PTR_RELEASE = "shared_ptr_release";
    constexpr const char* SHARED_PTR_USE_COUNT = "shared_ptr_use_count";
    constexpr const char* SMART_PTR_MALLOC = "smart_ptr_malloc";
    constexpr const char* WEAK_PTR_RELEASE = "weak_ptr_release";
}