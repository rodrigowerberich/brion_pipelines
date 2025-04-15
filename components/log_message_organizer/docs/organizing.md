# Organizing pipeline messages {#Organizing}

Organizing is divided into two parts:
- Spliting the messages by pipeline
    - split_by_pipeline.h
    - split_by_pipeline.cc
- Ordering and organizing the messages of the same pipeline:
    - organize_by_id.h
    - organize_by_id.cc


## Spliting the messages by pipeline

They are separated into different maps based on the pipeline id.

## Possible types of ids and references

At first glance you could imagine that the pipeline log could be represented as a chain of elements that end at one element that points at -1. 
But when you take into account the possible types of error that can exist it becomes a bit more complex.
You could have:
- Multiple messages with the same id
- Messages that point to themselves
- Messages that point to an id that does not exist

To better undersdand the implications of this lets take a look at the possible configuration a list can have, let's consider that elements that point to an invalid id and elements that point to nothing are the same.

We will use the notation x,y where x is the current id and y the next one.

If you have 1 element, you have two options:
@dot
digraph one_node {

  n0 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,-1"]
  b [label = "a,a"]
  subgraph cluster_0 {
    a->n0 
    label = "1";
  }
  
subgraph cluster_1 {
    b->b
    label = "2";
  }
  label = "One node";
}
@enddot

If you have 2 elements, you have 9 options:

@dot
digraph two_nodes {

  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,-1"]
  b [label = "b,-1"]
  c [label = "a,a"]
  d [label = "b,-1"]
  e [label = "a,a"]
  f [label = "b,b"]
  g [label = "a,b"]
  h [label = "b,-1"]
  i [label = "a,b"]
  j [label = "b,b"]
  k [label = "a,b"]
  l [label = "b,a"]
  
  subgraph cluster_0 {
    a->n0
    b->n0
    label = "1";
  }
  
  subgraph cluster_1 {
    c->c
    d->n1
    label = "2";
  }
  
  subgraph cluster_2 {
    e->e
    f->f
    label = "3";
  }
  
  subgraph cluster_3 {
    g->h
    h->n2
    label = "4";
  }  
  
  subgraph cluster_4 {
    i->j
    j->j
    label = "5";
  }  
  
  subgraph cluster_5 {
    k->l
    l->k
    label = "6";
  }  
  
  label = "Two nodes";
}
@enddot

With 3 more if you take cases 1, 2 and 3 and have them have the same ids. 

This already show many problematic scenarios such as detached lists and cyclic references. How should we order them?

## Organizing algorithm

To try and solve this a couple of choices were made.

1) All termination nodes, pointing to -1, will be put at the front of the list.

2) All nodes that point to invalid ids, will be after the termination nodes, as they are in a sense, also termination points.

3) The rest of the nodes will follow, maintaning when possible (cyclic dependencies) the inverse order of direction.

Let's look at one example to understand it better:


### Example
@dot

digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)"]
  c [label = "c,e"]
  d [label = "d,(e,m)"]
  e [label = "e,-1"]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
}

@enddot

Consider that the notation `b,(c,d,d)` represents three messages with id `b`, one pointing to `c`, another to `d`, and a third one pointing to `d` again. The line with `m` represents a message pointing to an index `m` which does not exist.

Following the rules we proposed the order of output would be something similar to:
```
(e,-1), (d,m), (d,e), (c,e), (b,c), (b,d), (b, d), (a,b)
```

To make that happen the following algorithm is followed:

1) We create 3 lists, one for general elements, one for termination elements and one for invalid id elements (ones that point to invalid ids).

2) Starting with any element we create a temporary chain list with the same 3 types of lists.

3) Add all elements with that id to the appropriate list, i.e., if it points to a valid element, to the general list, if it points to a termination, to the termination and if it points to an invalid element, to the invalid element list.

4) Mark this id as visited and follow the valid links that are not marked as visited. 

5) Repeat step 3 until you have added all elements to the temporary chain list

6) Add the list to general chain and then repeat from step 2 until you have no elements that are not marked as visited.

Seeing it in action:

We start with with "a random" id, let's say `b`. Id `b`, has 3 elements, they all point to valid ids, so they are all added one after each other to the temporary `b` general list.

The elements in id `b` points to two (actually 3, but one of them is repeated) ids. `C` and `d`.

So let's mark `b` as visited and go to `c`.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightblue,style=filled]
  c [label = "c,e"]
  d [label = "d,(e,m)"]
  e [label = "e,-1"]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  
  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    b0->b1->b2
    label = "B general list";
  }
}
@enddot

`C` only has one element, and it points to a valid element. So it goes in the temporary `c` general list. Let's mark id `c` as valid and go to id `e`.

@dot

digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightgreen,style=filled]
  c [label = "c,e", color=lightblue,style=filled]
  d [label = "d,(e,m)"]
  e [label = "e,-1"]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  
  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    b0->b1->b2
    label = "B general list";
  }
  
  subgraph cluster_1 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    c0 [label="(c,e)"]
    b0
    label = "C general list";
  }
}

@enddot

`e` also only has one element, but it points to the termination id. So it goes into the temporary `e` termination list.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightgreen,style=filled]
  c [label = "c,e", color=lightgreen,style=filled]
  d [label = "d,(e,m)"]
  e [label = "e,-1", color=lightblue,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  
  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    b0->b1->b2
    label = "B general list";
  }
  
  subgraph cluster_1 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    c0 [label="(c,e)"]
    label = "C general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d0 [label="(e,-1)"]
    label = "E termination list";
  }
}
@enddot

Since, `e` has no next id, we now merge all temporary elements from `e` to the temporary lists from `c`.
So now c has two lists: a general list and a termination list.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightgreen,style=filled]
  c [label = "c,e", color=darkorange,style=filled]
  d [label = "d,(e,m)"]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  
  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    b0->b1->b2
    label = "B general list";
  }
  
  subgraph cluster_C{
  subgraph cluster_1 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    c0 [label="(c,e)"]
    label = "C general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d0 [label="(e,-1)"]
    label = "C termination list";
  }
  }
}
@enddot

Now we merge the lists from `c` into the list from `b`. For general list, we add the descendent merging list into the original ending of the `b` general list. This ensures that descendent order is guaranteed.

For termination lists and invalid lists, the order does not matter.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=darkorange,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)"]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    b0->b1->b2->c0
    label = "B general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d0 [label="(e,-1)"]
    label = "B termination list";
  }
  }
}
@enddot

Now we go to the next descendent from `b`, which is `d`. `d` has two elements. One on the general list, the other in the invalid list.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightgreen,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightblue,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    b0->b1->b2->c0
    label = "B general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    e0 [label="(e,-1)"]
    label = "B termination list";
  }
  }
  
 subgraph cluster_D{  
  subgraph cluster_D1 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d0 [label="(d,e)"]
    d0
    label = "D general list";
  }
  
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d1 [label="(d, m)"]
    label = "D invalid list";
  }
  }
}
@enddot

Merging the lists from `d` into `b` yields the result below. Notice how `(d,e)` went before `(c,e)`. Like said before, new chain are added at the end of the original chain. This guarantess for instance that if `e` instead of pointing to the termination, pointed to another node `k`, that node `k` would be after `d` as it should, as it is an indirect descendent.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=darkorange,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightgrey,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    d0 [label="(d,e)"]
    b0->b1->b2->d0->c0
    label = "B general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    e0 [label="(e,-1)"]
    label = "B termination list";
  }
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d1 [label="(d, m)"]
    label = "B invalid list";
  }
  
  
  }
}
@enddot

Now we are done with `b`, so we merge `b`'s lists into the global list. And search for a new unvisited node.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b"]
  b [label = "b,(c,d,d)", color=lightgrey,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightgrey,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    d0 [label="(d,e)"]
    b0->b1->b2->d0->c0
    label = "Global general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    e0 [label="(e,-1)"]
    label = "Global termination list";
  }
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d1 [label="(d, m)"]
    label = "Global invalid list";
  }
  
  
  }
}
@enddot
`a` is the next one, it only has one element, and that goes in the temporary `a` general list. The descendent from `a` is already marked as visit, so we are done.
@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b", color=lightblue,style=filled]
  b [label = "b,(c,d,d)", color=lightgrey,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightgrey,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    d0 [label="(d,e)"]
    b0->b1->b2->d0->c0
    label = "Global general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    e0 [label="(e,-1)"]
    label = "Global termination list";
  }
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d1 [label="(d, m)"]
    label = "Global invalid list";
  }
  
  
  }
  
    subgraph cluster_a{  
      subgraph cluster_a1 {
        style=filled;
        color=lightgrey;
        node [style=filled,color=white];
        a0 [label="(a, b)"]
        label = "A general list";
      }
    }
}
@enddot

`a`'s list is merged into the global list. It follows the same logic as merging to another list. The new list needs to be added at the original end of the old list. Since the original global list, that is the same as adding the new chain to the beginning.

@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b", color=lightgrey,style=filled]
  b [label = "b,(c,d,d)", color=lightgrey,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightgrey,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    d0 [label="(d,e)"]
    a0 [label="(a, b)"]
    a0->b0->b1->b2->d0->c0
    label = "Global general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    e0 [label="(e,-1)"]
    label = "Global termination list";
  }
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    d1 [label="(d, m)"]
    label = "Global invalid list";
  }
 }
}
@enddot
We then append the invalid list and termination list, in this order, to the general list. And if we invert this order, we will have the desired organization of messages!
@dot
digraph G {
  rankdir="LR";
  n0 [label= "", shape=none,height=.0,width=.0]
  n1 [label= "", shape=none,height=.0,width=.0]
  n2 [label= "", shape=none,height=.0,width=.0]
  a [label = "a,b", color=lightgrey,style=filled]
  b [label = "b,(c,d,d)", color=lightgrey,style=filled]
  c [label = "c,e", color=lightgrey,style=filled]
  d [label = "d,(e,m)", color=lightgrey,style=filled]
  e [label = "e,-1", color=lightgrey,style=filled]
  
  a->b
  b->c
  b->d
  b->d
  c->e
  d->e
  d->n0 [label = m]
  e->n1
  

  subgraph cluster_C{  
  subgraph cluster_0 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    b0 [label="(b,c)"]
    b1 [label="(b,d)"]
    b2 [label="(b,d)"]
    c0 [label="(c,e)"]
    d0 [label="(d,e)"]
    a0 [label="(a, b)"]
    e0 [label="(e,-1)"]
    a0->b0->b1->b2->d0->c0->d1->e0
    d1 [label="(d, m)"]
    label = "Global general list";
  }
  
  subgraph cluster_2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    label = "Global termination list";
  }
  subgraph cluster_D2 {
    style=filled;
    color=lightgrey;
    node [style=filled,color=white];
    label = "Global invalid list";
  }
 }
}
@enddot

If you want to see more examples, please check the unit tests for the organize_by_id.cc