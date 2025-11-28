#!/usr/bin/env python3
"""
Analyze sp.h to find all functions that call sp_alloc (directly or transitively).
Uses libclang for AST-based analysis.
"""

import clang.cindex
from clang.cindex import CursorKind, Index
from collections import defaultdict
from typing import Dict, Set, List

def get_function_name(cursor) -> str:
    """Get the fully qualified name of a function."""
    return cursor.spelling

def build_call_graph(translation_unit) -> Dict[str, Set[str]]:
    """
    Build a call graph from the translation unit.
    Returns a dict mapping function_name -> set of functions it calls.
    """
    call_graph = defaultdict(set)
    current_function = None

    def visit(cursor, parent_function=None):
        nonlocal current_function

        # Track when we enter a function definition
        if cursor.kind == CursorKind.FUNCTION_DECL:
            # Only process if it has a body (is a definition, not just declaration)
            if cursor.is_definition():
                current_function = get_function_name(cursor)
                # Recurse into children with this function as context
                for child in cursor.get_children():
                    visit(child, current_function)
                current_function = None
                return

        # Track function calls
        if cursor.kind == CursorKind.CALL_EXPR:
            if parent_function:
                called_fn = cursor.spelling
                if called_fn:  # Some calls might not have a spelling
                    call_graph[parent_function].add(called_fn)

        # Recurse into children
        for child in cursor.get_children():
            visit(child, parent_function)

    visit(translation_unit.cursor)
    return call_graph

def find_callers(call_graph: Dict[str, Set[str]], target: str) -> Dict[str, Set[str]]:
    """
    Build reverse call graph: function -> set of functions that call it.
    """
    reverse_graph = defaultdict(set)
    for caller, callees in call_graph.items():
        for callee in callees:
            reverse_graph[callee].add(caller)
    return reverse_graph

def find_all_paths_to_target(reverse_graph: Dict[str, Set[str]], target: str) -> List[List[str]]:
    """
    Find all paths from callers to the target function.
    Returns list of paths, where each path is [caller, ..., target].
    """
    all_paths = []

    def dfs(current: str, path: List[str], visited: Set[str]):
        if current in visited:
            return

        new_path = path + [current]
        callers = reverse_graph.get(current, set())

        if not callers:
            # This is a root caller - add the path
            all_paths.append(new_path)
        else:
            visited.add(current)
            for caller in callers:
                dfs(caller, new_path, visited.copy())

    dfs(target, [], set())
    return all_paths

def find_transitive_callers(reverse_graph: Dict[str, Set[str]], target: str) -> Dict[str, int]:
    """
    Find all functions that call target directly or transitively.
    Returns dict mapping function -> depth (1 = direct caller, 2 = calls a direct caller, etc.)
    """
    callers = {}
    queue = [(target, 0)]
    visited = {target}

    while queue:
        current, depth = queue.pop(0)

        for caller in reverse_graph.get(current, []):
            if caller not in visited:
                visited.add(caller)
                callers[caller] = depth + 1
                queue.append((caller, depth + 1))

    return callers

def print_call_chains(reverse_graph: Dict[str, Set[str]], target: str, callers: Dict[str, int]):
    """
    Print call chains from each caller down to the target.
    """
    def get_chain(fn: str, visited: Set[str]) -> List[str]:
        """Get the shortest chain from fn to target."""
        if fn == target:
            return [fn]

        if fn in visited:
            return []

        visited.add(fn)

        # Find callees of this function that lead to target
        best_chain = None
        for callee, callers_of_callee in reverse_graph.items():
            if fn in callers_of_callee:
                # fn calls callee
                if callee == target or callee in callers:
                    chain = get_chain(callee, visited.copy())
                    if chain:
                        candidate = [fn] + chain
                        if best_chain is None or len(candidate) < len(best_chain):
                            best_chain = candidate

        return best_chain if best_chain else []

    # Group by depth
    by_depth = defaultdict(list)
    for fn, depth in callers.items():
        by_depth[depth].append(fn)

    print(f"\n{'='*60}")
    print(f"CALL GRAPH ANALYSIS: Functions calling '{target}'")
    print(f"{'='*60}\n")

    for depth in sorted(by_depth.keys()):
        fns = sorted(by_depth[depth])
        print(f"\n--- Depth {depth} ({len(fns)} functions) ---")
        for fn in fns:
            chain = get_chain(fn, set())
            if chain:
                print(f"  {' -> '.join(chain)}")
            else:
                print(f"  {fn} -> ... -> {target}")

def main():
    # Initialize libclang
    index = Index.create()

    # Parse sp.h - use C mode with common includes
    # We need to handle the header-only library correctly
    args = [
        '-x', 'c',  # Parse as C
        '-std=c11',
        '-DSP_IMPLEMENTATION',  # Enable implementation section
        '-I/usr/include',
    ]

    print("Parsing sp.h with libclang...")
    try:
        tu = index.parse(
            'sp.h',
            args=args,
            options=clang.cindex.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD
        )
    except Exception as e:
        print(f"Error parsing: {e}")
        return

    # Check for parse errors
    has_errors = False
    for diag in tu.diagnostics:
        if diag.severity >= clang.cindex.Diagnostic.Error:
            print(f"Parse error: {diag.spelling}")
            has_errors = True

    if has_errors:
        print("\nNote: There were parse errors. Results may be incomplete.")

    print("Building call graph...")
    call_graph = build_call_graph(tu)

    print(f"Found {len(call_graph)} functions with calls.")

    # Build reverse graph
    reverse_graph = find_callers(call_graph, 'sp_alloc')

    # Find all transitive callers of sp_alloc
    callers = find_transitive_callers(reverse_graph, 'sp_alloc')

    if not callers:
        print("\nNo callers of 'sp_alloc' found.")
        print("\nDirect call graph entries containing 'alloc':")
        for fn, callees in sorted(call_graph.items()):
            alloc_callees = [c for c in callees if 'alloc' in c.lower()]
            if alloc_callees:
                print(f"  {fn} calls: {', '.join(alloc_callees)}")
        return

    print(f"\nFound {len(callers)} functions that call 'sp_alloc' directly or transitively.")

    # Print organized output
    print_call_chains(reverse_graph, 'sp_alloc', callers)

    # Also print a summary sorted by function name
    print(f"\n{'='*60}")
    print("SUMMARY: All callers sorted alphabetically")
    print(f"{'='*60}\n")

    for fn in sorted(callers.keys()):
        depth = callers[fn]
        depth_str = "direct" if depth == 1 else f"depth {depth}"
        print(f"  {fn} ({depth_str})")

if __name__ == '__main__':
    main()
