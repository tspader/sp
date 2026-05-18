terraform {
  required_version = ">= 1.5"
  required_providers {
    github = {
      source  = "integrations/github"
      version = "~> 6.0"
    }
  }
}

provider "github" {
  owner = "tspader"
}

resource "github_repository" "sp" {
  name            = "sp"
  description     = "A modern C standard library"
  has_issues      = true
  has_projects    = true
  has_wiki        = true
  has_discussions = true

  lifecycle {
    prevent_destroy = true
  }
}

resource "github_actions_secret" "vouch_app_id" {
  repository      = github_repository.sp.name
  secret_name     = "VOUCH_APP_ID"
  value =var.vouch_app_id
}

resource "github_actions_secret" "vouch_app_private_key" {
  repository      = github_repository.sp.name
  secret_name     = "VOUCH_APP_PRIVATE_KEY"
  value =var.vouch_app_private_key
}

resource "github_repository_ruleset" "main" {
  name        = "main"
  repository  = github_repository.sp.name
  target      = "branch"
  enforcement = "active"

  conditions {
    ref_name {
      include = ["~DEFAULT_BRANCH"]
      exclude = []
    }
  }

  bypass_actors {
    actor_id    = 5 # RepositoryRole: admin
    actor_type  = "RepositoryRole"
    bypass_mode = "always"
  }

  bypass_actors {
    actor_id    = var.vouch_app_id
    actor_type  = "Integration"
    bypass_mode = "always"
  }

  rules {
    deletion         = true
    non_fast_forward = true
  }
}
