# infra/proto

## infra/proto vs chromite/infra/proto

This repository exists in two locations in the tree: `infra/proto`,
and `chromite/infra/proto`. The `infra/proto` repository is always at ToT,
while the `chromite/infra/proto` checkout is branched to allow the proto there
to (more) accurately reflect the version of the proto the Build API is using.

When making changes to the proto that you need to test in the Build API, you
will need ensure the changes are applied to the `chromite/infra/proto` checkout.
Chromite generates its proto bindings from the `chromite/infra/proto` repo.
