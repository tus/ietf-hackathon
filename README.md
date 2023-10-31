# IETF 118 Hackathon

The Internet Engineering Task Force (IETF) is holding a hackathon to encourage
developers and subject matter experts to discuss, collaborate, and develop
utilities, ideas, sample code, and solutions that show practical implementations
of IETF standards.

This is the repository for the [Resumable Uploads for HTTP][rufh] (RUFH)
hackathon project to hold the context and individual challenges. The project is
beginner friendly for those who are new to the IETF or new to the Hackathon.

## Prerequisites

- [The IETF hackathon wiki][wiki]
- [The Resumable Uploads for HTTP draft][rufh] (RUFH)
- Awareness of [tus][]. RUFH is an evolution of [tus][], an existing open
  protocol for resumable uploads. As such you can look at practical
  implementations, such as the official [tus-js-client][] and [tusd][], a
  reference implementation in Go with [support for the new IETF
  protocol][tusd-draft].
- Familiarity with either JavaScript (runtimes), HTTP proxies, or building CLI
  tools

## Challenges

### Resumability polyfill for the Fetch API in browsers

In the current state of Resumable Uploads for HTTP, we would still need a small
library to use it in the browser as the [Fetch][] API does not support resumable
uploads. Fetch falls under the WHATWG and there is an [issue][tus-fetch] to
discuss supporting resumable uploads but there is no timeline on that front.

#### Solution

To move things along and to have a plug-and-play alternative, we want to explore
creating a [polyfill][] for the [Fetch][] API for resumable uploads.

#### Requirements

- A new `resumable` option (`boolean`, default: `false`). Open to alternative
  APIs.

```js
const response = await fetch(url, {
  method: "POST",
  body: file,
  resumable: true,
});
```

- When the request completes the response should be provided
- When the request is interrupted and no `104 Upload Resumption Supported` was
  received, an error should be thrown.
- When the request is interrupted and a `104 Upload Resumption Supported` was
  received, the client should initiate a resumption. This can result in an error
  or response.

### Plugins for popular HTTP proxies for handling resumable uploads

In some cases people would like to handle files themselves through regular
requests rather than having a separate (or integrable) RUFH server, which would
write it to disk or to some storage provider. But they do want reliable,
resumable uploads without reading the files again yourself.

In other cases a platform, such as WordPress, can not be extended to handle
resumable uploads but could receive a traditional request with a file.

#### Solution

Plugins for popular HTTP proxies, which handle resumability with buffering. Once
the upload is complete, it will forward the upload as a regular HTTP request to
your backend.

[Nginx][] and [HAProxy][] seem like a good place to start, but we‘re open to
other contributions.

### CLI program to test if a server conforms to the spec

Implementing RUFH (or [tus][]) means re-implementing the same kind of end-to-end
tests over and over again.

#### Solution

A CLI program which runs end-to-end tests against a server.

#### Requirements

- Lightweight. Little or no dependencies.
- Single binary. People should not have to install a language and package
  manager to get the program, such as needing a `package.json` with Node.js
- Flags to only run subsets of tests so you can incrementally build your server
  to test against
- Optional: separate built-in tests for [tus][] v1. tus already has a big
  ecosystem of clients and servers and users such as Cloudfare, Supabase, Vimeo,
  and many others. Every server implementation has to write their own end-to-end
  tests, which is sometimes done well but not always. The ecosystem would
  greatly benefit from a single source of truth for tests.

### JavaScript runtime compatibility

There is a rise in JavaScript runtimes, such as Deno, Bun, Cloudfare Workers,
AWS Lambda, and more. These should have minimal differences, but the devil is in
the details and we need to know if the current version of RUFH works in most
runtimes, particularly on the edge.

#### Solution

A minimal proof of concept of RUFH in JavaScript with different adapters for
runtimes.

<!-- definitions -->

[rufh]: https://datatracker.ietf.org/doc/draft-ietf-httpbis-resumable-upload/02/
[wiki]: https://wiki.ietf.org/en/meeting/118/hackathon
[tus]: https://tus.io/
[tus-js-client]: https://github.com/tus/tus-js-client/
[tusd]: https://github.com/tus/tusd/
[tusd-draft]:
  https://tus.io/blog/2023/09/20/tusd-200#support-for-the-new-ietf-protocol
[nginx]: https://www.nginx.com/
[haproxy]: https://www.haproxy.com/
[fetch]: https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API
[tus-fetch]: https://github.com/whatwg/fetch/issues/1626
[polyfill]: https://developer.mozilla.org/en-US/docs/Glossary/Polyfill
